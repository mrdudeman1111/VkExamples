#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/importerdesc.h>
#include <assimp/material.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>

#include <sail/sail.h>
#include <sail-common/sail-common.h>
#include <sail-manip/sail-manip.h>

#ifndef NDEBUG
  std::vector<const char*> InstLayers = {"VK_LAYER_KHRONOS_validation"}; // VK_LAYER_KHRONOS_VALIDATION so we can get debug info in CLI
#else
  std::vector<const char*> InstLayers = {};
#endif

std::vector<const char*> InstExt = {"VK_KHR_surface"}; // VK_KHR_surface so we can interface with the surface, surface is a fancy image used for presentation to a display in this case.
std::vector<const char*> DevLayers = {};
std::vector<const char*> DevExt = {"VK_KHR_swapchain"}; // VK_KHR_swapchain so that we can create and use Swapchain objects to handle presentation to screen.

VkResult Error;

struct QueueFamily
{
  uint32_t Graphics, Compute;
};

struct DeviceMemory
{
  // these represent the indices to the specified memory
  uint32_t Local = -1;
  uint32_t Visible = -1;
};

struct
{
  VkInstance Instance;

  VkPhysicalDevice PhysicalDevice;
  QueueFamily Families;
  DeviceMemory Memory;

  VkDevice Device;
  VkQueue GraphicsQueue;
  VkQueue ComputeQueue;

  VkExtent2D WindowExtent;
  GLFWwindow* Window;
  VkSurfaceKHR Surface;

  uint32_t FrameBufferCount;
  VkSurfaceFormatKHR SurfaceFormat;

  VkSwapchainKHR Swapchain;
  std::vector<VkImage> SwapchainImages;
  std::vector<VkImageView> SwapchainImageViews;
  std::vector<VkFramebuffer> FrameBuffers;

  std::vector<VkDeviceMemory> DepthMemory;
  std::vector<VkImage> DepthImages;
  std::vector<VkImageView> DepthImageViews;

  VkRenderPass Renderpass;

  VkCommandPool GraphicsPool;
  std::vector<VkCommandBuffer> RenderBuffers;
  VkCommandBuffer MemoryCmd;

  VkShaderModule VertexShader;
  VkShaderModule FragmentShader;

  VkPipeline Pipeline;
  VkPipelineLayout PipelineLayout;

  // in vulkan, Buffers have no memory until they are bound to an actual memory address, which is where their data is stored.
  VkDeviceMemory TransitMemory;
  VkBuffer TransitBuffer;

  VkDeviceMemory MeshVertexMemory;
  VkBuffer MeshVertexBuffer;

  VkDeviceMemory MeshIndexMemory;
  VkBuffer MeshIndexBuffer;

  VkFence MemoryFence = VK_NULL_HANDLE;

  // used to wait on Image Acquisition (wait for an available image to render to)
  VkFence ImageFence = VK_NULL_HANDLE;

  // one fence for each renderbuffer/framebuffer so we can check if they are done before we use it again
  std::vector<VkFence> CmdFence;

  // used to Synchronize render submitions, so we don't overlap render operations (this will lead to corrupted framebuffers and lag)
  VkSemaphore RenderSemaphore = VK_NULL_HANDLE;


  /*
    Acquire next image()
    - ImageFence: Signal when Image is acquired

    Submit render buffer for Image
    - ImageFence: Wait for Image to be acquired

    - RenderSemaphore: Signal RenderSemaphore when finished rendering

    Present Image()
    - RenderSemaphore: Wait for the image to be rendered.
  */

  // Descriptor Pool for our WVP Buffer and Sampler image
  VkDescriptorPool DescPool;

  // WVP Layout
  VkDescriptorSetLayout DescLayout;
  // Sampler layout
  VkDescriptorSetLayout SamplerLayout;

  // WVP Set
  VkDescriptorSet DescSet;

  // WVP Buffer
  VkBuffer mvpBuffer;
  VkDeviceMemory mvpMemory;

  // Mesh Importer
  Assimp::Importer Importer;
} Context;
// contains all the vulkan structures that will be used to render.

void InitInstance();
void InitPhysicalDevice();
void InitDevice();
void InitSwapchain(VkExtent2D WindowExtent);
void InitRenderpass();
void InitFrameBuffer();
void InitCommandPool();
void InitCommandBuffer();
void InitShaders();
void InitPipeline();
void InitMesh();
void InitDescriptorPool();
void InitDescriptors();
void InitSamplers();
void InitSync();

void SubmitRender(uint32_t RenderIndex);
void Present(uint32_t RenderIndex);

void UpdateDescriptors();
void RecordRender(uint32_t ImageIndex);

void CreateImage(VkImage* Image, VkDeviceMemory* Memory, VkFormat Format, VkImageLayout ImageLayout, VkImageUsageFlags Flags);
void CreateImageView(VkImage* Image, VkImageView* View, VkFormat Format, VkImageLayout ImageLayout, VkImageUsageFlags Usage);

void Cleanup();

// Packed Struct containing our MVP matrices
struct WVP
{
  glm::mat4 World;
  glm::mat4 View;
  glm::mat4 Proj;
};

// Camera class
class Camera
{
  public:
    WVP Matrices;

    Camera()
    {
      Position = glm::vec3(0.f, 0.f, 5.f);
      Rotation = glm::vec3(0.f, 0.f, 0.f);

      Matrices.World = glm::mat4(1.f);

      Matrices.Proj = glm::perspective(glm::radians(45.f), 16.f/9.f, 0.1f, 100.f);
      Matrices.Proj[1][1] *= -1;

      Matrices.View = glm::inverse(glm::translate(glm::mat4(1.f), Position));

      CameraMatrix = glm::mat4(1.f);
    }

    void Update()
    {
      // X is left/right
      // Y is up/down
      // Z is forward/back
      // per GLSL standard

      /*
       * When we rotate along an axis it changes the local axis. We have to be aware of this and therefore enforce an "order of transformations".
       * We will always rotate along the Y (up/down) axis first.
       * We will then rotate along the X (right left) axis AFTER the Y rotation next.
       * We will finally rotate along the Z (forward/back) axis AFTER the Z and X rotations.
       * If you want to test different orders, reorder these calls and definitions, but don't delete them.
      */

      // Find The Change in mouse position.
      if(WindowFocused)
      {
        double X, Y;
        glfwGetCursorPos(Context.Window, &X, &Y);

        double DeltaX, DeltaY;

        DeltaX = X-LastMouse.x;
        DeltaY = Y-LastMouse.y;

        LastMouse.x = X;
        LastMouse.y = Y;

        Rotation.y -= (DeltaX*RotateSpeed);
        Rotation.x -= (DeltaY*RotateSpeed);

        Rotation.x = std::clamp(Rotation.x, -90.f, 90.f);
      }

      glm::vec3 RightVector = glm::vec3(CameraMatrix[0][0], CameraMatrix[0][1], CameraMatrix[0][2]);
      glm::vec3 UpVector = glm::vec3(CameraMatrix[1][0], CameraMatrix[1][1], CameraMatrix[1][2]);
      glm::vec3 ForwardVector = (-1.f*glm::vec3(CameraMatrix[2][0], CameraMatrix[2][1], CameraMatrix[2][2]));

      float MovementModifier;

      // calculate keyboard inputs.
      {
        if(glfwGetKey(Context.Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
          MovementModifier = 2.f;
        }
        else
        {
          MovementModifier = 1.f;
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_W) == GLFW_PRESS)
        {
          Position += (ForwardVector*MoveSpeed*MovementModifier);
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_S) == GLFW_PRESS)
        {
          Position += (ForwardVector*MoveSpeed*MovementModifier*-1.f);
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_D) == GLFW_PRESS)
        {
          Position += (RightVector*MoveSpeed*MovementModifier);
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_A) == GLFW_PRESS)
        {
          Position += (RightVector*MoveSpeed*MovementModifier*-1.f);
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_E) == GLFW_PRESS)
        {
          Position += (UpVector*MoveSpeed*MovementModifier);
        }
        if(glfwGetKey(Context.Window, GLFW_KEY_Q) == GLFW_PRESS)
        {
          Position += (UpVector*MoveSpeed*MovementModifier*-1.f);
        }
      }

      // Translate Camera Matrix based on glm::vec3 Position
      CameraMatrix = glm::translate(glm::mat4(1.f), Position);

      // Rotate Camera Matrix based on glm::vec3 Rotation
      {
        glm::vec3 LookRightVector = glm::vec3(CameraMatrix[0][0], CameraMatrix[0][1], CameraMatrix[0][2]);
        glm::vec3 LookUpVector = glm::vec3(0.f, 1.f, 0.f);
        glm::vec3 LookForwardVector = (-1.f*glm::vec3(CameraMatrix[2][0], CameraMatrix[2][1], CameraMatrix[2][2]));

        // The Rotation axis is the global Y
        CameraMatrix = glm::rotate(CameraMatrix, glm::radians(Rotation.y), LookUpVector);

        CameraMatrix = glm::rotate(CameraMatrix, glm::radians(Rotation.x), LookRightVector);

        CameraMatrix = glm::rotate(CameraMatrix, glm::radians(Rotation.z), LookForwardVector);

        Matrices.View = glm::inverse(CameraMatrix);
      }

      if(glfwGetKey(Context.Window, GLFW_KEY_TAB) == GLFW_PRESS)
      {
        if(WindowFocused)
        {
          WindowFocused = false;
          glfwSetInputMode(Context.Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
          WindowFocused = true;
          glfwSetInputMode(Context.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
      }
    }

  private:
    glm::mat4 CameraMatrix;

    glm::vec3 Position;
    glm::vec3 Rotation;

    glm::vec2 LastMouse;

    bool WindowFocused = true;

    float MoveSpeed = 0.005f;
    float RotateSpeed = 0.01f;
} Camera;

// Heads up, classes aren't packed like structs, so things might get messy if you try to define the Vertetx as a class instead of a struct.
struct Vertex
{
  public:
    glm::vec3 vPos;
    glm::vec3 vColor;
    glm::vec2 vCoord;
};

struct
{
  public:
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;

    VkImage Texture_Albedo;
    VkImageView View_Albedo;
    VkDeviceMemory Memory_Albedo;

    VkSampler Sampler_Albedo;
    VkDescriptorSet TextureDescriptor;
} Mesh;

/* we will now put our vertex input description's bindings/attributes so we can see the changes better */
std::vector<VkVertexInputBindingDescription> GetBindingDescription()
{
  std::vector<VkVertexInputBindingDescription> Ret(1);

  Ret[0].stride = sizeof(Vertex);
  Ret[0].binding = 0;
  Ret[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return Ret;
}

std::vector<VkVertexInputAttributeDescription> GetAttributeDescription()
{
  std::vector<VkVertexInputAttributeDescription> Ret(3);

  Ret[0].binding = 0;
  Ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[0].location = 0;
  Ret[0].offset = offsetof(Vertex, vPos);

  Ret[1].binding = 0;
  Ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[1].location = 1;
  Ret[1].offset = offsetof(Vertex, vColor);

  Ret[2].binding = 0;
  Ret[2].format = VK_FORMAT_R32G32_SFLOAT;
  Ret[2].location = 2;
  Ret[2].offset = offsetof(Vertex, vCoord);

  return Ret;
}

// Helper Functions
void CreateImage(VkImage* Image, VkDeviceMemory* Memory, VkExtent2D Resolution, VkFormat Format, VkImageLayout ImageLayout, VkImageUsageFlags Usage)
{
  VkImageCreateInfo ImageCI{};
  ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ImageCI.extent = VkExtent3D{Resolution.width, Resolution.height, 1};
  ImageCI.format = Format;
  ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  ImageCI.usage = Usage;
  ImageCI.imageType = VK_IMAGE_TYPE_2D;
  ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  ImageCI.mipLevels = 1;
  ImageCI.arrayLayers = 1;
  ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ImageCI.initialLayout = ImageLayout;

  if((Error = vkCreateImage(Context.Device, &ImageCI, nullptr, Image)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image with error: " + std::to_string(Error));
  }

  VkMemoryRequirements MemReq;
  vkGetImageMemoryRequirements(Context.Device, *Image, &MemReq);

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.allocationSize = MemReq.size;
  AllocInfo.memoryTypeIndex = Context.Memory.Local;


  if((Error = vkAllocateMemory(Context.Device, &AllocInfo, nullptr, Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate image error: " + std::to_string(Error));
  }

  if((Error = vkBindImageMemory(Context.Device, *Image, *Memory, 0)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to bind image memory with error: " + std::to_string(Error));
  }
}

void CreateImageView(VkImage* Image, VkImageView* View, VkFormat Format, VkImageAspectFlags Aspects)
{
  VkImageViewCreateInfo ViewCI{};
  ViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ViewCI.format = Format;
  ViewCI.image = *Image;

  ViewCI.subresourceRange.levelCount = 1;
  ViewCI.subresourceRange.layerCount = 1;

  ViewCI.subresourceRange.aspectMask = Aspects;

  ViewCI.subresourceRange.baseArrayLayer = 0;
  ViewCI.subresourceRange.baseMipLevel = 0;

  ViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
  ViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
  ViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
  ViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

  ViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

  if((vkCreateImageView(Context.Device, &ViewCI, nullptr, View)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image view with error: " + std::to_string(Error));
  }
}

void CopyBuffer(VkBuffer* Src, VkBuffer* Dst, VkBufferCopy* CopyInfo)
{
  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
    vkCmdCopyBuffer(Context.MemoryCmd, *Src, *Dst, 1, CopyInfo);
  vkEndCommandBuffer(Context.MemoryCmd);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

  if((Error = vkQueueSubmit(Context.GraphicsQueue,  1, &SubmitInfo, Context.MemoryFence)) == VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit Buffer to Buffer copy with error: " + std::to_string(Error));
  }
}

/*
 * in VkBuffer* Src  Pointer to source buffer, should be TransitBuffer
 * in VkImage* Dst   Pointer to destination Image
 * in VkImageLayout Layout  Layout of destination image
 * in VkBufferImageCopy CopyInfo  Struct specifying memory ranges to copy
*/
void CopyBufferImage(VkBuffer* Src, VkImage* Dst, VkImageLayout Layout, VkBufferImageCopy CopyInfo)
{
  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
    vkCmdCopyBufferToImage(Context.MemoryCmd, *Src, *Dst, Layout, 1, &CopyInfo);
  vkEndCommandBuffer(Context.MemoryCmd);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

  if((Error = vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit Buffer to Image copy with error: " + std::to_string(Error));
  }
}


int main()
{
  // this function initiates our Window Library, so we can create a window on the screen with one function later.
  glfwInit();

  // Initate Instance.
  // The Instance is a handle to this application's vulkan state.
  // It also verifies that all Requested Instance Layers and extensions are available.
  // if available it assembles an instance call chain. for more info Look into
  // VkInstance
  // VkInstanceCreateInfo
  // vkCreateInstance
  // vulkan Loader
  // Vulkan Trampoline
  // Vulkan Instance call chain
  InitInstance();

  // Find Dedicated GPU, also query Queue Families.
  // If we need to get Hardware Info (Like VRAM Size, Bus width, etc)
  // we can do that at any point in the program, after we find the GPU we want to use.
  InitPhysicalDevice();

  // Initiate a logical Device.
  // The Device is a handle to the applications device state and connection to the physical device.
  // Similar to the Instance Initiation, but instad validates Device Instances and Extensions.
  // if available, the call Proceeds to assemble the device call chain.
  InitDevice();

  // these are the window dimensions we will use for the program.
  VkExtent2D WindowSize = {1280, 720};

  // Create Surface 
  // Create Swapchain
  InitSwapchain(WindowSize);

  // Create primary renderpass
  // with one subpass for mesh rendering
  InitRenderpass();

  // Init frambuffer with color attachment (screen)
  // and depth attachment (depth stencil for facial culling)
  InitFrameBuffer();

  // Init Command pool for GPU processes like copies and rendering
  InitCommandPool();

  // Init Command Buffers for memory operations and rendering operations
  InitCommandBuffer();

  // Read and create shaders for pipeline
  InitShaders();

  // create main pipeline with shaders from before
  InitPipeline();

  // Create structures needed for execution synchronization
  // so that we have memory safety
  InitSync();

  // Initiate mesh vertices, indices, colors, coordinates, and textures.
  InitMesh();

  // Initiate descriptor pool for passing buffers and textures to shaders.
  InitDescriptorPool();

  // initiate descriptors for buffers, like the camera buffer
  InitDescriptors();

  // Initiate descriptors for Textures, like the Mesh Albedo texture
  InitSamplers();

  uint32_t ImageIndex;

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwSetInputMode(Context.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Render loop
  while(!glfwWindowShouldClose(Context.Window))
  {
    glfwPollEvents();
    Camera.Update();
    UpdateDescriptors();

    vkAcquireNextImageKHR(Context.Device, Context.Swapchain, UINT64_MAX, VK_NULL_HANDLE, Context.ImageFence, &ImageIndex);

    vkWaitForFences(Context.Device, 1, &Context.ImageFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.ImageFence);

    vkWaitForFences(Context.Device, 1, &Context.CmdFence[ImageIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.CmdFence[ImageIndex]);

    RecordRender(ImageIndex);

    SubmitRender(ImageIndex);

    Present(ImageIndex);

    //vkWaitForFences(Context.Device, 1, &Context.CmdFence[ImageIndex], VK_TRUE, UINT64_MAX);
  }

  vkDeviceWaitIdle(Context.Device);

  Cleanup();

  std::cout << "Succesfull run\n";
  return 0;
}

void InitInstance()
{
  // usually, this is not required in a vulkan application, but we are using glfw to make Window management crossplatform and easy
  {
    uint32_t glfwExtCount = -1;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    assert(glfwExtCount != -1);

    for(uint32_t i = 0; i < glfwExtCount; i++)
    {
      InstExt.push_back(glfwExtensions[i]);
    }
  }

  VkInstanceCreateInfo InstCI{};
  InstCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  InstCI.enabledLayerCount = InstLayers.size();
  InstCI.ppEnabledLayerNames = InstLayers.data();
  InstCI.enabledExtensionCount = InstExt.size();
  InstCI.ppEnabledExtensionNames = InstExt.data();

  if((Error = vkCreateInstance(&InstCI, nullptr, &Context.Instance)) != VK_SUCCESS)
  {
    // if the Instance creation fails, throw the Error valaue. We can use that value to see what VkResult Enum value was thrown.
    throw std::runtime_error("failed to create Instance with error " + std::to_string(Error));
  }
}

void InitPhysicalDevice()
{
  // Find Dedicated GPU.
  {
    // Retrieve list of all available Graphical Hardware.
    uint32_t PhysicalDeviceCount;
    vkEnumeratePhysicalDevices(Context.Instance, &PhysicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> Devices(PhysicalDeviceCount);
    vkEnumeratePhysicalDevices(Context.Instance, &PhysicalDeviceCount, Devices.data());

    // Enumerate over list until we find GPU.
    for(uint32_t i = 0; i < PhysicalDeviceCount; i++)
    {
      VkPhysicalDeviceProperties DevProps;
      vkGetPhysicalDeviceProperties(Devices[i], &DevProps);

      if(DevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        Context.PhysicalDevice = Devices[i];
      }
    }
  }

  // Find all the Queue Families. You might want to read up on this real quick
  {
    // Retrieve list of queue families.
    uint32_t QFamPropCount;
    vkGetPhysicalDeviceQueueFamilyProperties(Context.PhysicalDevice, &QFamPropCount, nullptr);
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QFamPropCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Context.PhysicalDevice, &QFamPropCount, QueueFamilyProperties.data());

    bool Graphics = false;
    bool Compute = false;

    // Enumerate over queue families until we find a Graphics family and a Compute family. (sometimes there are several)
    for(uint32_t i = 0; i < QFamPropCount; i++)
    {
      if(QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !Graphics)
      {
        Context.Families.Graphics = i;
      }
      else if(QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !Compute)
      {
        Context.Families.Compute = i;
      }
    }
  }

  // Find The Host visible and device local memory
  {
    VkPhysicalDeviceMemoryProperties MemProps;

    vkGetPhysicalDeviceMemoryProperties(Context.PhysicalDevice, &MemProps);

    bool Local = false, Visible = false;

    for(uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
    {
      if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && !Local)
      {
        Context.Memory.Local = i;
        Local = true;
        std::cout << "Local Properties " << MemProps.memoryTypes[i].propertyFlags << '\n';
      }
      else if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && !Visible)
      {
        Context.Memory.Visible = i;
        Visible = true;
        std::cout << "Visibile properties " << MemProps.memoryTypes[i].propertyFlags << '\n';
      }
    }

    assert(Context.Memory.Local != -1);
    assert(Context.Memory.Visible != -1);
  }
}

void InitDevice()
{
  // Initiate elements with Defaults(0)
  VkDeviceQueueCreateInfo QInfo[2]{};
  // 0 = Graphics
  // 1 = Compute

  // Queue Info
  {
    // VkDeviceQueueCreateInfo::sType
    QInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    // one queue each
    QInfo[0].queueCount = 1;
    QInfo[1].queueCount = 1;

    // Specify queue family 
    // Kind of like a queue Type, 
    // GraphicsFamily will be for graphics commands, rendering, and presentation
    // ComputeFamily will be for compute commands, processing info, and memory operations.
    // There are also Transfer, Sparse Binding, Protected, Video Decoding, Video Encoding, and Optical Flow Queues.
    QInfo[0].queueFamilyIndex = Context.Families.Graphics;
    QInfo[1].queueFamilyIndex = Context.Families.Compute;
  }

  // Create Device and queues
  {
    VkDeviceCreateInfo DeviceInfo{};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceInfo.enabledExtensionCount = DevExt.size();
    DeviceInfo.ppEnabledExtensionNames = DevExt.data();
    DeviceInfo.queueCreateInfoCount = 2;
    DeviceInfo.pQueueCreateInfos = QInfo;

    if((Error = vkCreateDevice(Context.PhysicalDevice, &DeviceInfo, nullptr, &Context.Device)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to Create Device " + std::to_string(Error));
    }
  }

  // Grab the returned Queues.
  {
    vkGetDeviceQueue(Context.Device, Context.Families.Graphics, 0, &Context.GraphicsQueue);
    vkGetDeviceQueue(Context.Device, Context.Families.Compute, 0, &Context.ComputeQueue);
  }
}

void InitSwapchain(VkExtent2D WindowExtent)
{
  Context.WindowExtent = WindowExtent;

  // Create a Window on the desktop, and a surface to send rendered images to.
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    Context.Window = glfwCreateWindow(WindowExtent.width, WindowExtent.height, "Hello Triangle", NULL, NULL);

    if((Error = glfwCreateWindowSurface(Context.Instance, Context.Window, nullptr, &Context.Surface)) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create window surface with error: " + std::to_string(Error));
    }
  }

  VkSurfaceCapabilitiesKHR SurfaceCapabilities;

  // Get Surface Format, and colorspace.
  {
    // We get the display's Swapchain Capabilities through the GPU.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context.PhysicalDevice, Context.Surface, &SurfaceCapabilities);

    // The Number of framebuffers we can have is dependent on the surface's capabilities.
    // so we will use MinimumFramebufferCount+1 (The +1 is good practice, you can exclude it, but it might result in instability and tearing)
    Context.FrameBufferCount = SurfaceCapabilities.minImageCount + 1;

    // Get Monitor Format
    {
      // Get display's available formats, again, through the GPU.
      uint32_t SurfaceFormatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(Context.PhysicalDevice, Context.Surface, &SurfaceFormatCount, nullptr);
      std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(Context.PhysicalDevice, Context.Surface, &SurfaceFormatCount, SurfaceFormats.data());

      // Enumerate over available surface formats until we find compatible format
      for(uint32_t i = 0; i < SurfaceFormatCount; i++)
      {
        // We want RGB output, just cuz. We could look for other formats, but then we would need to adjust some formulas, and I don't want to do that.
        if(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
          Context.SurfaceFormat = SurfaceFormats[i];
        }
      }
    }
  }

  // CreateSwapchain
  {
    VkSwapchainCreateInfoKHR SwapchainCI{};
    SwapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCI.pNext = nullptr;
    SwapchainCI.flags = 0;

    SwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainCI.imageExtent = WindowExtent;
    SwapchainCI.imageFormat = Context.SurfaceFormat.format;
    SwapchainCI.imageColorSpace = Context.SurfaceFormat.colorSpace;
    SwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainCI.imageArrayLayers = 1;

    SwapchainCI.clipped = VK_FALSE;
    SwapchainCI.surface = Context.Surface;
    SwapchainCI.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    SwapchainCI.minImageCount = Context.FrameBufferCount;

    SwapchainCI.preTransform = SurfaceCapabilities.currentTransform;

    // what do we do when the window on the desktop, is placed over another window.
    // In this case, we want to occlude the window behind ours. So we will use Opaque;
    SwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    if(vkCreateSwapchainKHR(Context.Device, &SwapchainCI, nullptr, &Context.Swapchain) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create swapchain");
    }
  }

  // Grab Swapchain Images for later.
  {
    uint32_t ImageCount;
    vkGetSwapchainImagesKHR(Context.Device, Context.Swapchain, &ImageCount, nullptr);

    // sometimes, the swapchain will make more images than it needs, for safety
    Context.FrameBufferCount = ImageCount;
    Context.SwapchainImages.resize(ImageCount);

    vkGetSwapchainImagesKHR(Context.Device, Context.Swapchain, &ImageCount, Context.SwapchainImages.data());
  }
}

void InitRenderpass()
{
  // Renderpasses are weird. They need to know what the framebuffers you use will look like. For example, maybe our framebuffers have color and depth images attached.
  // The renderpass will need to know that, however, You can't have framebuffers without first creating the renderpass. So the framebuffers will not exist yet,
  // So you will need to know in advance, what will be in your framebuffer, Additionally, you will need to create structures that specify what to do with each image during rendering.

  VkAttachmentDescription ColorDescription{};
  VkAttachmentReference ColorReference{};
  VkAttachmentDescription DepthDescription{};
  VkAttachmentReference DepthReference{};

  // Color Description
  {
    ColorDescription.format = Context.SurfaceFormat.format;
    ColorDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    ColorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

    // This is very inefficient code, When the Image Layout is specified as undefined, it must be changed at load. This introduces more complexity in the renderpass
    // leading to slower render times. Later, we will add something called "Pipeline Barriers" and "Image Memory Barriers"
    ColorDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  }

  // Color Reference
  {
    // this is the layout AFTER the Subpass and BEFORE the renderpass ends.
    ColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // this is the first and only attachment in the framebuffer, so it's at index 0
    ColorReference.attachment = 0;
  }

  // Depth Description
  {
    DepthDescription.format = VK_FORMAT_D32_SFLOAT;
    DepthDescription.samples = VK_SAMPLE_COUNT_1_BIT;

    DepthDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    DepthDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  }

  // Depth Reference
  {
    DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    DepthReference.attachment = 1;
  }

  VkAttachmentDescription Descriptions[] = {ColorDescription, DepthDescription};

  // Create Subpass and Renderpass
  {
    VkSubpassDescription SubpassDescription{};
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColorReference;
    SubpassDescription.pDepthStencilAttachment = &DepthReference;

    VkRenderPassCreateInfo RPInfo{};
    RPInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    RPInfo.subpassCount = 1;
    RPInfo.pSubpasses = &SubpassDescription;

    RPInfo.attachmentCount = 2;
    RPInfo.pAttachments = Descriptions;

    if((Error = vkCreateRenderPass(Context.Device, &RPInfo, nullptr, &Context.Renderpass)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create renderpass with error: " + std::to_string(Error));
    }
  }
}

void InitFrameBuffer()
{
  // Create Swapchain Image views
  {
    Context.SwapchainImageViews.resize(Context.SwapchainImages.size());

    Context.DepthImages.resize(Context.SwapchainImages.size());
    Context.DepthImageViews.resize(Context.SwapchainImages.size());
    Context.DepthMemory.resize(Context.SwapchainImageViews.size());

    // Create the Image view for our Swapchain Image
    for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
    {
      {
        VkImageCreateInfo DepthCI{};
        DepthCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        DepthCI.format = VK_FORMAT_D32_SFLOAT;
        DepthCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        DepthCI.extent = VkExtent3D{Context.WindowExtent.width, Context.WindowExtent.height, 1};
        DepthCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        DepthCI.samples = VK_SAMPLE_COUNT_1_BIT;
        DepthCI.imageType = VK_IMAGE_TYPE_2D;
        DepthCI.arrayLayers = 1;
        DepthCI.mipLevels = 1;
        DepthCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        DepthCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if((Error = vkCreateImage(Context.Device, &DepthCI, nullptr, &Context.DepthImages[i])) != VK_SUCCESS)
        {
          throw std::runtime_error("Failed to create an Depth image with error: " + std::to_string(Error));
        }

        VkMemoryRequirements DepthMemReq;
        vkGetImageMemoryRequirements(Context.Device, Context.DepthImages[i], &DepthMemReq);

        VkMemoryAllocateInfo DepthAllocInfo{};
        DepthAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        DepthAllocInfo.allocationSize = DepthMemReq.size;
        DepthAllocInfo.memoryTypeIndex = Context.Memory.Local;

        if((Error = vkAllocateMemory(Context.Device, &DepthAllocInfo, nullptr, &Context.DepthMemory[i])) != VK_SUCCESS)
        {
          throw std::runtime_error("Failed to allocate depth memory with error: " + std::to_string(Error));
        }

        if((Error = vkBindImageMemory(Context.Device, Context.DepthImages[i], Context.DepthMemory[i], 0)) != VK_SUCCESS)
        {
          throw std::runtime_error("Failed to bind depth image memory with error: " + std::to_string(Error));
        }
      }

      VkImageViewCreateInfo DepthViewCI{};
      DepthViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      DepthViewCI.format = VK_FORMAT_D32_SFLOAT;
      DepthViewCI.image = Context.DepthImages[i];
      DepthViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

      DepthViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      DepthViewCI.subresourceRange.layerCount = 1;
      DepthViewCI.subresourceRange.levelCount = 1;
      DepthViewCI.subresourceRange.baseMipLevel = 0;
      DepthViewCI.subresourceRange.baseArrayLayer = 0;

      DepthViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
      DepthViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
      DepthViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
      DepthViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

      if((Error = vkCreateImageView(Context.Device, &DepthViewCI, nullptr, &Context.DepthImageViews[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create depth image view with error: " + std::to_string(Error));
      }

      VkImageViewCreateInfo ViewInfo{};
      ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      ViewInfo.format = Context.SurfaceFormat.format;
      ViewInfo.image = Context.SwapchainImages[i];
      ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

      ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      ViewInfo.subresourceRange.layerCount = 1;
      ViewInfo.subresourceRange.levelCount = 1;
      ViewInfo.subresourceRange.baseMipLevel = 0;
      ViewInfo.subresourceRange.baseArrayLayer = 0;

      ViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
      ViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
      ViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;

      if((Error = vkCreateImageView(Context.Device, &ViewInfo, nullptr, &Context.SwapchainImageViews[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create image view with error " + std::to_string(Error));
      }
    }
  }

  // Create FrameBuffers.
  {
    // Resize the Frame buffer array in preperation for creation.
    Context.FrameBuffers.resize(Context.FrameBufferCount);

    for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
    {
      VkImageView ImageViews[] = {Context.SwapchainImageViews[i], Context.DepthImageViews[i]};

      VkFramebufferCreateInfo FBInfo{};
      FBInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

      FBInfo.attachmentCount = 2;
      FBInfo.pAttachments = ImageViews;

      FBInfo.renderPass = Context.Renderpass;

      FBInfo.width = Context.WindowExtent.width;
      FBInfo.height = Context.WindowExtent.height;

      FBInfo.layers = 1;

      if((Error = vkCreateFramebuffer(Context.Device, &FBInfo, nullptr, &Context.FrameBuffers[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create framebuffer with error: " + std::to_string(Error));
      }
    }
  }
}

void InitCommandPool()
{
  VkCommandPoolCreateInfo PoolCreateInfo{};
  PoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  PoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  PoolCreateInfo.queueFamilyIndex = Context.Families.Graphics;

  if(vkCreateCommandPool(Context.Device, &PoolCreateInfo, nullptr, &Context.GraphicsPool) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create graphics pool with error: " + std::to_string(Error));
  }
}

void InitCommandBuffer()
{
  Context.RenderBuffers.resize(Context.FrameBufferCount);

  // allocate the command buffers
  {
    for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
    {
      VkCommandBufferAllocateInfo RenderBufferInfo{};
      RenderBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      RenderBufferInfo.commandPool = Context.GraphicsPool;
      RenderBufferInfo.commandBufferCount = 1;
      RenderBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

      if((Error = vkAllocateCommandBuffers(Context.Device, &RenderBufferInfo, &Context.RenderBuffers[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate command buffer " + std::to_string(i) + " with error: " + std::to_string(Error));
      }
    }

    VkCommandBufferAllocateInfo MemoryCmdInfo{};
    MemoryCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    MemoryCmdInfo.commandPool = Context.GraphicsPool;
    MemoryCmdInfo.commandBufferCount = 1;;
    MemoryCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if((Error = vkAllocateCommandBuffers(Context.Device, &MemoryCmdInfo, &Context.MemoryCmd)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate memory command buffer with error: " + std::to_string(Error));
    }
  }
}

void InitShaders()
{
  std::ifstream Vertex;
  std::ifstream Fragment;
  uint32_t VertexSize;
  uint32_t FragmentSize;
  char* VertexCode;
  char* FragmentCode;

  // read the shader code from the .spv file
  {
    Vertex.open("Vert.spv", std::ifstream::ate);
    Fragment.open("Frag.spv",  std::ifstream::ate);

    assert(Vertex.is_open());
    assert(Fragment.is_open());

    VertexSize = Vertex.tellg();
    FragmentSize = Fragment.tellg();

    Vertex.seekg(0);
    Fragment.seekg(0);

    VertexCode = new char[VertexSize];
    FragmentCode = new char[FragmentSize];

    Vertex.read(VertexCode, VertexSize);
    Fragment.read(FragmentCode, FragmentSize);
  }

  // Assemble the vertex module
  {
    VkShaderModuleCreateInfo VertexCreateInfo{};
    VertexCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VertexCreateInfo.codeSize = VertexSize;
    VertexCreateInfo.pCode = reinterpret_cast<uint32_t*>(VertexCode);

    if((Error = vkCreateShaderModule(Context.Device, &VertexCreateInfo, nullptr, &Context.VertexShader)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create vertex shader with errror: " + std::to_string(Error));
    }
  }

  // Assemble the Fragment module
  {
    VkShaderModuleCreateInfo FragmentCreateInfo{};
    FragmentCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    FragmentCreateInfo.codeSize = FragmentSize;
    FragmentCreateInfo.pCode = reinterpret_cast<uint32_t*>(FragmentCode);

    if((Error = vkCreateShaderModule(Context.Device, &FragmentCreateInfo, nullptr, &Context.FragmentShader)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create fragment shader with error: " + std::to_string(Error));
    }
  }
}

void InitPipeline()
{
  VkDescriptorSetLayoutBinding UniformBinding{};

  // setup descriptor binding for the MVP and albedo sampler
  {
    VkDescriptorSetLayoutBinding UniformBinding{};
    UniformBinding.descriptorCount = 1;
    UniformBinding.binding = 0;
    UniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    UniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = 1;
    LayoutCI.pBindings = &UniformBinding;

    if((Error = vkCreateDescriptorSetLayout(Context.Device, &LayoutCI, nullptr, &Context.DescLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Error));
    }
  }

  // Setup descriptor binding for the Sampler
  {
    VkDescriptorSetLayoutBinding ImageBinding{};
    ImageBinding.descriptorCount = 1;
    ImageBinding.binding = 0;
    ImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    ImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = 1;
    LayoutCI.pBindings = &ImageBinding;

    if((Error = vkCreateDescriptorSetLayout(Context.Device, &LayoutCI, nullptr, &Context.SamplerLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Error));
    }
  }

  VkDescriptorSetLayout Layouts[] = {Context.DescLayout, Context.SamplerLayout};

  // Create the pipeline layout
  {
    VkPipelineLayoutCreateInfo PipeLayoutInfo{};
    PipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipeLayoutInfo.setLayoutCount = 2;
    PipeLayoutInfo.pSetLayouts = Layouts;
    PipeLayoutInfo.pushConstantRangeCount = 0;
    PipeLayoutInfo.pPushConstantRanges = nullptr;

    if((Error = vkCreatePipelineLayout(Context.Device, &PipeLayoutInfo, nullptr, &Context.PipelineLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create pipeline layout with error: " + std::to_string(Error));
    }
  }

  VkRect2D Scissor{};
  Scissor.extent.width = 1280;
  Scissor.extent.height = 720;
  Scissor.offset.x = 0;
  Scissor.offset.y = 0;

  VkViewport Viewport{};
  Viewport.y = 0;
  Viewport.x = 0;
  Viewport.width = 1280;
  Viewport.height = 720;
  Viewport.minDepth = 0;
  Viewport.maxDepth = 1;

  VkPipelineViewportStateCreateInfo PipelineViewport{};

  // Viewport
  {
    PipelineViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    PipelineViewport.scissorCount = 1;
    PipelineViewport.pScissors = &Scissor;
    PipelineViewport.viewportCount = 1;
    PipelineViewport.pViewports = &Viewport;
  }

  VkPipelineInputAssemblyStateCreateInfo InputAssembly{};

  // InputAssembly (topology type, Index type, etc)
  {
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;
  }

  VkPipelineShaderStageCreateInfo PipelineShaderStages[2]{};

  // Shader stages (Vertex, Fragment)
  {
    PipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    PipelineShaderStages[0].pName = "main";
    PipelineShaderStages[0].module = Context.VertexShader;

    PipelineShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    PipelineShaderStages[1].pName = "main";
    PipelineShaderStages[1].module = Context.FragmentShader;
  }

  VkPipelineColorBlendAttachmentState ColorAttachment{};
  VkPipelineColorBlendStateCreateInfo ColorBlendState{};

  // Color blending
  {
    ColorAttachment.blendEnable = VK_FALSE;
    ColorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    ColorBlendState.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendState.logicOpEnable = VK_FALSE;

    ColorBlendState.blendConstants[0] = 0.f;
    ColorBlendState.blendConstants[1] = 0.f;
    ColorBlendState.blendConstants[2] = 0.f;
    ColorBlendState.blendConstants[3] = 0.f;

    ColorBlendState.attachmentCount = 1;
    ColorBlendState.pAttachments = &ColorAttachment;
  }

  VkPipelineVertexInputStateCreateInfo VertexInput{};
    std::vector<VkVertexInputAttributeDescription> VertexAttribute;
    std::vector<VkVertexInputBindingDescription> VertexBinding;

  // Setup Vertex Input
  {
    VertexAttribute = GetAttributeDescription();
    VertexBinding = GetBindingDescription();

    VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInput.vertexBindingDescriptionCount = VertexBinding.size();
    VertexInput.vertexAttributeDescriptionCount = VertexAttribute.size();
    VertexInput.pVertexBindingDescriptions = VertexBinding.data();
    VertexInput.pVertexAttributeDescriptions = VertexAttribute.data();
  }

  VkPipelineDepthStencilStateCreateInfo DepthState{};

  // Depth stencil testing
  {
    DepthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthState.depthCompareOp = VK_COMPARE_OP_GREATER;
    DepthState.minDepthBounds = 0.f;
    DepthState.maxDepthBounds = 1.f;
    DepthState.depthTestEnable = VK_TRUE;
    DepthState.depthWriteEnable = VK_FALSE;
    DepthState.stencilTestEnable = VK_TRUE;
    DepthState.depthBoundsTestEnable = VK_FALSE;
  }

  VkPipelineRasterizationStateCreateInfo RasterState{};

  // Rasterization Info
  {
    RasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterState.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterState.lineWidth = 1.f;
    RasterState.polygonMode = VK_POLYGON_MODE_FILL;

    RasterState.depthBiasEnable = VK_FALSE;
    RasterState.depthClampEnable = VK_FALSE;
  }

  VkPipelineMultisampleStateCreateInfo SampleState{};

  // MultiSample settings for pipelines
  {
    SampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    SampleState.minSampleShading = 1.f;
    SampleState.alphaToOneEnable = false;
    SampleState.alphaToCoverageEnable = VK_FALSE;
    SampleState.sampleShadingEnable = VK_FALSE;
    SampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  }

  VkGraphicsPipelineCreateInfo PipelineCI{};

  // Pipeline Creation
  {
    PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCI.layout = Context.PipelineLayout;
    PipelineCI.renderPass = Context.Renderpass;
    PipelineCI.subpass = 0;
    PipelineCI.stageCount = 2;
    PipelineCI.pStages = PipelineShaderStages;
    PipelineCI.pInputAssemblyState = &InputAssembly;
    PipelineCI.pViewportState = &PipelineViewport;
    PipelineCI.pColorBlendState = &ColorBlendState;
    PipelineCI.pVertexInputState = &VertexInput;
    PipelineCI.pDepthStencilState = &DepthState;
    PipelineCI.pRasterizationState = &RasterState;
    PipelineCI.pMultisampleState = &SampleState;
  }

  if((Error = vkCreateGraphicsPipelines(Context.Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Context.Pipeline)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Graphics pipeline with error: " + std::to_string(Error));
  }
}

void InitMesh()
{
  // Initiate Transfer Buffer
  {
    VkBufferCreateInfo tBufferInfo{};
    tBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    tBufferInfo.size = 512000000; // 512 MB
    tBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Error = vkCreateBuffer(Context.Device, &tBufferInfo, nullptr, &Context.TransitBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create transit buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements tMemReq{};
    vkGetBufferMemoryRequirements(Context.Device, Context.TransitBuffer, &tMemReq);

    VkMemoryAllocateInfo tMemAlloc{};
    tMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tMemAlloc.allocationSize = tMemReq.size;
    tMemAlloc.memoryTypeIndex = Context.Memory.Visible;

    if((Error = vkAllocateMemory(Context.Device, &tMemAlloc, nullptr, &Context.TransitMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate memory for transit buffer with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(Context.Device, Context.TransitBuffer, Context.TransitMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind transit buffer to transit memory with error: " + std::to_string(Error));
    }
  }

  // Load Mesh file
  const aiScene* pScene = Context.Importer.ReadFile("Mesh.dae", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

  // check for meshes, if none, then throw error
  if(pScene->HasMeshes())
  {
    // Prepare vertex array
    Mesh.Vertices.resize(pScene->mMeshes[0]->mNumVertices);

    // Fill Vertex array and vertex colors if there are any
    for(uint32_t i = 0; i < pScene->mMeshes[0]->mNumVertices; i++)
    {
      Mesh.Vertices[i].vPos.x = pScene->mMeshes[0]->mVertices[i].y;
      Mesh.Vertices[i].vPos.y = pScene->mMeshes[0]->mVertices[i].z;
      Mesh.Vertices[i].vPos.z = -1.f*pScene->mMeshes[0]->mVertices[i].x;

      if(pScene->mMeshes[0]->mTextureCoords[0])
      {
        // account for opengl vs vulkan texture coordinates by inverting the y coordinate
        Mesh.Vertices[i].vCoord.x = pScene->mMeshes[0]->mTextureCoords[0][i].x;
        Mesh.Vertices[i].vCoord.y = -1*pScene->mMeshes[0]->mTextureCoords[0][i].y;
      }

      if(pScene->mMeshes[0]->HasVertexColors(0))
      {
        Mesh.Vertices[i].vColor.r = pScene->mMeshes[0]->mColors[0][i].r;
        Mesh.Vertices[i].vColor.g = pScene->mMeshes[0]->mColors[0][i].g;
        Mesh.Vertices[i].vColor.b = pScene->mMeshes[0]->mColors[0][i].b;
      }
    }

    // Fill Index Array
    for(uint32_t i = 0; i < pScene->mMeshes[0]->mNumFaces; i++)
    {
      // if the number of Indices isn't 3. Then it's not a triangle, we don't want that.
      // the pipeline topology type is VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST meaning we can only assemble triangles
      assert(pScene->mMeshes[0]->mFaces[i].mNumIndices == 3);

      Mesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[0]);
      Mesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[1]);
      Mesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[2]);
    }

    // Load textures
    {
      aiString AlbedoPath;
      pScene->mMaterials[pScene->mMeshes[0]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &AlbedoPath);

      sail_image *RawImage, *Image;
      std::cout << AlbedoPath.C_Str() << '\n';
      sail_load_from_file(AlbedoPath.C_Str(), &RawImage);

      sail_convert_image_with_options(RawImage, SAIL_PIXEL_FORMAT_BPP32_RGBA, NULL, &Image);

      CreateImage(&Mesh.Texture_Albedo, &Mesh.Memory_Albedo, VkExtent2D{Image->width, Image->height}, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      CreateImageView(&Mesh.Texture_Albedo, &Mesh.View_Albedo, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

      // Layout Transition
      {
        VkImageMemoryBarrier ImgBarrier{};
        ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        ImgBarrier.image = Mesh.Texture_Albedo;
        ImgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ImgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        ImgBarrier.srcAccessMask = VK_ACCESS_NONE;
        ImgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        ImgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImgBarrier.subresourceRange.layerCount = 1;
        ImgBarrier.subresourceRange.levelCount = 1;
        ImgBarrier.subresourceRange.baseMipLevel = 0;
        ImgBarrier.subresourceRange.baseArrayLayer = 0;

        VkCommandBufferBeginInfo BeginInfo{};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

          vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
            vkCmdPipelineBarrier(Context.MemoryCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImgBarrier);
          vkEndCommandBuffer(Context.MemoryCmd);

        VkSubmitInfo SubmitInfo{};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        SubmitInfo.commandBufferCount = 1;
        SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

        vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence);

        vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);
        vkResetFences(Context.Device, 1, &Context.MemoryFence);
      }

      // Copy buffer to image
      {
        void* pTransit;
        vkMapMemory(Context.Device, Context.TransitMemory, 0, 512000000, 0, &pTransit);
        memcpy((uint32_t*)pTransit, (uint32_t*)Image->pixels, (Image->width*Image->height*sizeof(uint32_t)));

        VkBufferImageCopy CopyInfo{};

        // fill copy info
        {
          CopyInfo.imageExtent = {(uint32_t)Image->width, (uint32_t)Image->height, 1};
          CopyInfo.imageOffset = VkOffset3D{0, 0, 0};
          CopyInfo.bufferOffset = 0;

          CopyInfo.imageSubresource.layerCount = 1;
          CopyInfo.imageSubresource.mipLevel = 0;
          CopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          CopyInfo.imageSubresource.baseArrayLayer = 0;
        }

        CopyBufferImage(&Context.TransitBuffer, &Mesh.Texture_Albedo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, CopyInfo);

        vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);
        vkResetFences(Context.Device, 1, &Context.MemoryFence);
      }

      vkUnmapMemory(Context.Device, Context.TransitMemory);
    }

    // Create vertex buffer
    {
      // Define Buffer
      VkBufferCreateInfo vBufferInfo{};
      vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      vBufferInfo.size = sizeof(Vertex)*Mesh.Vertices.size();
      vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      // Create buffer
      if((Error = vkCreateBuffer(Context.Device, &vBufferInfo, nullptr, &Context.MeshVertexBuffer)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create triangle vertex buffer with error: " + std::to_string(Error));
      }

      // Get alignment, size, and memory type requirements
      VkMemoryRequirements vMemReq;
      vkGetBufferMemoryRequirements(Context.Device, Context.MeshVertexBuffer, &vMemReq);

      // Define memory allocation
      VkMemoryAllocateInfo vAllocInfo{};
      vAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      vAllocInfo.pNext = nullptr;
      vAllocInfo.allocationSize = vMemReq.size;
      vAllocInfo.memoryTypeIndex = Context.Memory.Local;

      // Allocate memory
      if((Error = vkAllocateMemory(Context.Device, &vAllocInfo, nullptr, &Context.MeshVertexMemory)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate Triangle Vertex memory with error: " + std::to_string(Error));
      }

      // Bind memory
      if((Error = vkBindBufferMemory(Context.Device, Context.MeshVertexBuffer, Context.MeshVertexMemory, 0)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to bind Triangle vertex buffer to memory with error: " + std::to_string(Error));
      }
    }

    // Repeat with Index buffer
    {
      VkBufferCreateInfo iBufferInfo{};
      iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      iBufferInfo.size = sizeof(uint32_t) * Mesh.Indices.size();
      iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      iBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if((Error = vkCreateBuffer(Context.Device, &iBufferInfo, nullptr, &Context.MeshIndexBuffer)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create Triangle Index buffer with error: " + std::to_string(Error));
      }

      VkMemoryRequirements iMemReq;
      vkGetBufferMemoryRequirements(Context.Device, Context.MeshIndexBuffer, &iMemReq);

      VkMemoryAllocateInfo iAllocInfo{};
      iAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      iAllocInfo.pNext = nullptr;
      iAllocInfo.allocationSize = iMemReq.size;
      iAllocInfo.memoryTypeIndex = Context.Memory.Local;

      if(iMemReq.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) std::cout << "Index buffer supports Local\n";

      if((Error = vkAllocateMemory(Context.Device, &iAllocInfo, nullptr, &Context.MeshIndexMemory)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate Triangle Index Memory with error: " + std::to_string(Error));
      }

      if((Error = vkBindBufferMemory(Context.Device, Context.MeshIndexBuffer, Context.MeshIndexMemory, 0)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to bind Triangle index memory with error: " + std::to_string(Error));
      }
    }

    std::cout << "Mesh loaded\n";
  }
  else
  {
    throw std::runtime_error("Tried to load mesh file with no meshes");
  }

  // Map Transit Memory
  void* TransitMemory;
  vkMapMemory(Context.Device, Context.TransitMemory, 0, 512000000, 0, &TransitMemory);

  // Fill Transit Buffer with Vertices
  {
    memcpy((Vertex*)TransitMemory, Mesh.Vertices.data(), sizeof(Vertex)*Mesh.Vertices.size());
  }

  // Transfer Vertices from Transit to Vertex Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(Vertex) * Mesh.Vertices.size();
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(Context.MemoryCmd, Context.TransitBuffer, Context.MeshVertexBuffer, 1, &CopyInfo);
    vkEndCommandBuffer(Context.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

    vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence);
  }

  // wait for transfer to finish
  {
    vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);

    vkResetFences(Context.Device, 1, &Context.MemoryFence);

    vkResetCommandBuffer(Context.MemoryCmd, 0);
  }

  // Fill Transfer buffer with Indices
  {
    memcpy((uint32_t*)TransitMemory, Mesh.Indices.data(), sizeof(uint32_t)*Mesh.Indices.size());
  }

  // Copy Indices from Transit to Index Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(uint32_t) * Mesh.Indices.size();
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(Context.MemoryCmd, Context.TransitBuffer, Context.MeshIndexBuffer, 1, &CopyInfo);
    vkEndCommandBuffer(Context.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

    vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence);
  }

  // Wait for transfer to finish
  {
    vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);

    vkResetFences(Context.Device, 1, &Context.MemoryFence);

    vkResetCommandBuffer(Context.MemoryCmd, 0);
  }

  vkUnmapMemory(Context.Device, Context.TransitMemory);
}

void InitDescriptorPool()
{
  // Uniform, Sampler
  VkDescriptorPoolSize DescriptorSizes[2];

  DescriptorSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  DescriptorSizes[0].descriptorCount = 1;

  DescriptorSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  DescriptorSizes[1].descriptorCount = 1;

  VkDescriptorPoolCreateInfo PoolInfo{};
  PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  PoolInfo.maxSets = 2; // One for the Uniform buffer, one for the Image sampler
  PoolInfo.poolSizeCount = 2;
  PoolInfo.pPoolSizes = DescriptorSizes;

  if((Error = vkCreateDescriptorPool(Context.Device, &PoolInfo, nullptr, &Context.DescPool)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create descriptor pool\n");
  }
}

void InitDescriptors()
{
  // allocate Uniform descriptor
  {
    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = Context.DescPool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &Context.DescLayout;

    if((Error = vkAllocateDescriptorSets(Context.Device, &AllocInfo, &Context.DescSet)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor sets with error: " + std::to_string(Error));
    }
  }

  // allocate Sampler descriptor
  {
    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = Context.DescPool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &Context.SamplerLayout;

    if((Error = vkAllocateDescriptorSets(Context.Device, &AllocInfo, &Mesh.TextureDescriptor)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create sampler set with error: " + std::to_string(Error));
    }
  }

  // allocate mvp buffer
  {
    VkBufferCreateInfo BufferCI{};
    BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCI.size = sizeof(WVP);
    BufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Error = vkCreateBuffer(Context.Device, &BufferCI, nullptr, &Context.mvpBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create MVP Buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements BufferMemoryRequirements;
    vkGetBufferMemoryRequirements(Context.Device, Context.mvpBuffer, &BufferMemoryRequirements);

    VkMemoryAllocateInfo MemAlloc{};
    MemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemAlloc.allocationSize = BufferMemoryRequirements.size;
    MemAlloc.memoryTypeIndex = Context.Memory.Local;

    if((Error = vkAllocateMemory(Context.Device, &MemAlloc, nullptr, &Context.mvpMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate memory with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(Context.Device, Context.mvpBuffer, Context.mvpMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind memory to mvp buffer with error: " + std::to_string(Error));
    }
  }
}

void InitSamplers()
{
  VkSamplerCreateInfo SamplerCI{};

  // settup Sampler create info
  {
    SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    SamplerCI.minLod = 1.f;
    SamplerCI.maxLod = 1.f;

    SamplerCI.minFilter = VK_FILTER_LINEAR;
    SamplerCI.magFilter = VK_FILTER_LINEAR;

    SamplerCI.maxAnisotropy = 4.f;
    SamplerCI.mipLodBias = 1.f;
    SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    SamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    SamplerCI.compareEnable = VK_FALSE;
    SamplerCI.anisotropyEnable = VK_FALSE;

    SamplerCI.unnormalizedCoordinates = VK_FALSE;
  }

  if((Error = vkCreateSampler(Context.Device, &SamplerCI, nullptr, &Mesh.Sampler_Albedo)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create mesh sampler with error: " + std::to_string(Error));
  }

  // Quickly Perform layout transition
  {
    VkImageMemoryBarrier ImgBarrier{};
    ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImgBarrier.image = Mesh.Texture_Albedo;
    ImgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    ImgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImgBarrier.srcAccessMask = VK_ACCESS_NONE;
    ImgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    ImgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImgBarrier.subresourceRange.layerCount = 1;
    ImgBarrier.subresourceRange.levelCount = 1;
    ImgBarrier.subresourceRange.baseMipLevel = 0;
    ImgBarrier.subresourceRange.baseArrayLayer = 0;

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
        vkCmdPipelineBarrier(Context.MemoryCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImgBarrier);
      vkEndCommandBuffer(Context.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

    vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence);

    vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.MemoryFence);
  }

  // Update sampler Texture
  {
    VkDescriptorImageInfo ImageWrite{};
    ImageWrite.sampler = Mesh.Sampler_Albedo; // Target sampler
    ImageWrite.imageView = Mesh.View_Albedo;
    ImageWrite.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet WriteInfo{};
    WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    WriteInfo.dstSet = Mesh.TextureDescriptor;
    WriteInfo.pImageInfo = &ImageWrite;

    WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteInfo.descriptorCount = 1;
    WriteInfo.dstArrayElement = 0;

    vkUpdateDescriptorSets(Context.Device, 1, &WriteInfo, 0, nullptr);
  }
}

void InitSync()
{
  VkSemaphoreCreateInfo SemInfo{};
  SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if((Error = vkCreateSemaphore(Context.Device, &SemInfo, nullptr, &Context.RenderSemaphore)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Rendering Semaphore with error: " + std::to_string(Error));
  }

  VkFenceCreateInfo FenceInfo{};
  FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if((Error = vkCreateFence(Context.Device, &FenceInfo, nullptr, &Context.MemoryFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create memory fence with error: " + std::to_string(Error));
  }

  if((Error = vkCreateFence(Context.Device, &FenceInfo, nullptr, &Context.ImageFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image fence with error: " + std::to_string(Error));
  }

  FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  Context.CmdFence.resize(Context.FrameBufferCount);

  for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
  {
    if((Error = vkCreateFence(Context.Device, &FenceInfo, nullptr, &Context.CmdFence[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create fence with error: " + std::to_string(Error));
    }
  }
}

void RecordRender(uint32_t ImageIndex)
{
  // record the render command for each frame buffer
  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.pNext = nullptr;

  VkClearValue ColorClearValue{};

  // Color Values
  ColorClearValue.color.int32[0] = 0; ColorClearValue.color.int32[1] = 0; ColorClearValue.color.int32[2] = 0; ColorClearValue.color.int32[3] = 0;

  VkClearValue DepthClearValue{};

  // Depth Values                          // Depth stencil index
  DepthClearValue.depthStencil.depth = 0.f; DepthClearValue.depthStencil.stencil = 1;

  // a clear value for each attachment
  VkClearValue ClearValues[] = {ColorClearValue, DepthClearValue};

  VkRenderPassBeginInfo RenderPassInfo{};
  RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  RenderPassInfo.pNext = nullptr;
  RenderPassInfo.renderPass = Context.Renderpass;

  RenderPassInfo.renderArea.extent = {1280, 720};
  RenderPassInfo.renderArea.offset = {0, 0};

  RenderPassInfo.clearValueCount = 2;
  RenderPassInfo.pClearValues = ClearValues;

  RenderPassInfo.framebuffer = Context.FrameBuffers[ImageIndex];

  vkBeginCommandBuffer(Context.RenderBuffers[ImageIndex], &BeginInfo);

    vkCmdBeginRenderPass(Context.RenderBuffers[ImageIndex], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkDeviceSize Offset = 0;

      // Bind the Mesh information and pipeline.
      vkCmdBindVertexBuffers(Context.RenderBuffers[ImageIndex], 0, 1, &Context.MeshVertexBuffer, &Offset);
      vkCmdBindIndexBuffer(Context.RenderBuffers[ImageIndex], Context.MeshIndexBuffer, Offset, VK_INDEX_TYPE_UINT32);
      vkCmdBindDescriptorSets(Context.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, Context.PipelineLayout, 0, 1, &Context.DescSet, 0, nullptr);
      vkCmdBindDescriptorSets(Context.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, Context.PipelineLayout, 1, 1, &Mesh.TextureDescriptor, 0, nullptr);
      vkCmdBindPipeline(Context.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, Context.Pipeline);

      // Draw the mesh
      vkCmdDrawIndexed(Context.RenderBuffers[ImageIndex], Mesh.Indices.size()/*as specified in Triangle's struct definition*/, 1, 0, 0, 0);

    vkCmdEndRenderPass(Context.RenderBuffers[ImageIndex]);

  vkEndCommandBuffer(Context.RenderBuffers[ImageIndex]);
}

void SubmitRender(uint32_t RenderIndex)
{
  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &Context.RenderBuffers[RenderIndex];

  // this semaphore will be used during presentation to ensure that the image has been completely finished before beginning rendering again
  SubmitInfo.signalSemaphoreCount = 1;
  SubmitInfo.pSignalSemaphores = &Context.RenderSemaphore;

  vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.CmdFence[RenderIndex]);
}

void Present(uint32_t RenderIndex)
{
  VkPresentInfoKHR PresentInfo{};
  PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  PresentInfo.waitSemaphoreCount = 1;
  PresentInfo.pWaitSemaphores = &Context.RenderSemaphore;

  PresentInfo.pResults = &Error;

  PresentInfo.pImageIndices = &RenderIndex;

  PresentInfo.swapchainCount = 1;
  PresentInfo.pSwapchains = &Context.Swapchain;

  vkQueuePresentKHR(Context.GraphicsQueue, &PresentInfo);
}

void UpdateDescriptors()
{
  // Update Buffer
  {
    void* TransferMemory;
    vkMapMemory(Context.Device, Context.TransitMemory, 0, sizeof(WVP), 0, &TransferMemory);

    memcpy(TransferMemory, &Camera.Matrices, sizeof(Camera.Matrices));


    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy Copy{};
    Copy.size = sizeof(Camera.Matrices);
    Copy.srcOffset = 0;
    Copy.dstOffset = 0;

    vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(Context.MemoryCmd, Context.TransitBuffer, Context.mvpBuffer, 1, &Copy);
    vkEndCommandBuffer(Context.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Context.MemoryCmd;

    vkQueueSubmit(Context.GraphicsQueue, 1, &SubmitInfo, Context.MemoryFence);
  }

  // Update Descriptor
  {
    VkDescriptorBufferInfo BuffInfo{};
    BuffInfo.range = sizeof(WVP);
    BuffInfo.offset = 0;
    BuffInfo.buffer = Context.mvpBuffer;

    VkWriteDescriptorSet WriteInfo{};
    WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    WriteInfo.descriptorCount = 1;

    WriteInfo.dstSet = Context.DescSet;
    WriteInfo.dstBinding = 0;

    WriteInfo.pBufferInfo = &BuffInfo;

    vkWaitForFences(Context.Device, 1, &Context.MemoryFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.MemoryFence);

    vkUpdateDescriptorSets(Context.Device, 1, &WriteInfo, 0, nullptr);
  }

  vkUnmapMemory(Context.Device, Context.TransitMemory);
}

void Cleanup()
{
  vkDestroySampler(Context.Device, Mesh.Sampler_Albedo, nullptr);

  vkDestroyImageView(Context.Device, Mesh.View_Albedo, nullptr);
  vkDestroyImage(Context.Device, Mesh.Texture_Albedo, nullptr);
  vkFreeMemory(Context.Device, Mesh.Memory_Albedo, nullptr);

  vkFreeCommandBuffers(Context.Device, Context.GraphicsPool, Context.RenderBuffers.size(), Context.RenderBuffers.data());
  vkFreeCommandBuffers(Context.Device, Context.GraphicsPool, 1, &Context.MemoryCmd);
  vkDestroyCommandPool(Context.Device, Context.GraphicsPool, nullptr);

  vkDestroySemaphore(Context.Device, Context.RenderSemaphore, nullptr);
  vkDestroyFence(Context.Device, Context.ImageFence, nullptr);
  vkDestroyFence(Context.Device, Context.MemoryFence, nullptr);

  for(uint32_t i = 0; i < Context.CmdFence.size(); i++)
  {
    vkDestroyFence(Context.Device, Context.CmdFence[i], nullptr);
  }

  vkDestroyBuffer(Context.Device, Context.TransitBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.TransitMemory, nullptr);

  vkDestroyBuffer(Context.Device, Context.MeshVertexBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.MeshVertexMemory, nullptr);

  vkDestroyBuffer(Context.Device, Context.MeshIndexBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.MeshIndexMemory, nullptr);

  vkDestroyBuffer(Context.Device, Context.mvpBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.mvpMemory, nullptr);

  // This will destroy all descriptor sets allocated as well
  vkDestroyDescriptorPool(Context.Device, Context.DescPool, nullptr);
  vkDestroyDescriptorSetLayout(Context.Device, Context.DescLayout, nullptr);
  vkDestroyDescriptorSetLayout(Context.Device, Context.SamplerLayout, nullptr);

  vkDestroyPipeline(Context.Device, Context.Pipeline, nullptr);
  vkDestroyPipelineLayout(Context.Device, Context.PipelineLayout, nullptr);

  vkDestroyShaderModule(Context.Device, Context.VertexShader, nullptr);
  vkDestroyShaderModule(Context.Device, Context.FragmentShader, nullptr);

  for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
  {
    vkDestroyFramebuffer(Context.Device, Context.FrameBuffers[i], nullptr);
  }

  for(uint32_t i = 0; i < Context.SwapchainImageViews.size(); i++)
  {
    vkDestroyImageView(Context.Device, Context.SwapchainImageViews[i], nullptr);
    vkDestroyImageView(Context.Device, Context.DepthImageViews[i], nullptr);
  }

  for(uint32_t i = 0; i < Context.SwapchainImageViews.size(); i++)
  {
    vkDestroyImage(Context.Device, Context.DepthImages[i], nullptr);
    vkFreeMemory(Context.Device, Context.DepthMemory[i], nullptr);
  }

  vkDestroyRenderPass(Context.Device, Context.Renderpass, nullptr);

  // this will also destroy our swapchain images.
  vkDestroySwapchainKHR(Context.Device, Context.Swapchain, nullptr);

  vkDestroySurfaceKHR(Context.Instance, Context.Surface, nullptr);

  glfwDestroyWindow(Context.Window);
  glfwTerminate();

  // this destroys our queues too
  vkDestroyDevice(Context.Device, nullptr);

  vkDestroyInstance(Context.Instance, nullptr);
}

