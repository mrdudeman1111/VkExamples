#include <cstdint>
#include <cstring>
#include <fstream>
#include <glm/fwd.hpp>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

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

  VkDeviceMemory TriangleVertexMemory;
  VkBuffer TriangleVertexBuffer;

  VkDeviceMemory TriangleIndexMemory;
  VkBuffer TriangleIndexBuffer;

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
void InitSync();
void RecordRender();
void SubmitRender(uint32_t RenderIndex);
void Present(uint32_t RenderIndex);

void Cleanup();

// Instance
// PhysicalDevice
// Device
// Queue
// Swapchain
// Subpass
// Renderpass
// Framebuffer
// CommandPool
// CommandBuffer
// Sem/fence
// Shaders
// Descriptors.
// Pipeline
// VBO, IBO.
// loop
  // Record Render commands
  // Present


// Heads up, classes aren't packed like structs, so things might get messy if you try to define the Vertetx as a class instead of a struct.
struct Vertex
{
  glm::vec2 vPos;
  glm::vec3 vColor;
};

struct
{
  Vertex Vertices[3] = 
  {
    {glm::vec2{0.5f, 0.5f}, glm::vec3(1.f, 0.f, 0.f)},
    {glm::vec2{-0.5f, 0.5f}, glm::vec3{0.f, 1.f, 0.f}},
    {glm::vec2{0.f, -0.5f}, glm::vec3{0.f, 0.f, 1.f}}
  };

  uint32_t Indices[3] = {2, 1, 0};

} TriangleMesh;

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
  std::vector<VkVertexInputAttributeDescription> Ret(2);

  Ret[0].binding = 0;
  Ret[0].format = VK_FORMAT_R32G32_SFLOAT;
  Ret[0].location = 0;
  Ret[0].offset = offsetof(Vertex, vPos);

  Ret[1].binding = 0;
  Ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[1].location = 1;
  Ret[1].offset = offsetof(Vertex, vColor);

  return Ret;
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

  InitRenderpass();

  InitFrameBuffer();

  InitCommandPool();

  InitCommandBuffer();

  InitShaders();

  InitPipeline();

  InitSync();

  InitMesh();

  RecordRender();

 uint32_t ImageIndex;

  // Render loop
  while(!glfwWindowShouldClose(Context.Window))
  {
    glfwPollEvents();

    vkAcquireNextImageKHR(Context.Device, Context.Swapchain, UINT64_MAX, VK_NULL_HANDLE, Context.ImageFence, &ImageIndex);

    vkWaitForFences(Context.Device, 1, &Context.ImageFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.ImageFence);

    vkWaitForFences(Context.Device, 1, &Context.CmdFence[ImageIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(Context.Device, 1, &Context.CmdFence[ImageIndex]);

    SubmitRender(ImageIndex);

    Present(ImageIndex);
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

  // Create Subpass and Renderpass
  {
    VkSubpassDescription SubpassDescription{};
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColorReference;

    VkRenderPassCreateInfo RPInfo{};
    RPInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    RPInfo.subpassCount = 1;
    RPInfo.pSubpasses = &SubpassDescription;

    RPInfo.attachmentCount = 1;
    RPInfo.pAttachments = &ColorDescription;

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

    // Create the Image view for our Swapchain Image
    for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
    {
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

    // We assign AttachmentCount to the number of Image Views associated each frame buffer.
    // To do this we need to take the total number of images and divide it by the number of framebuffers we have.
    uint32_t AttachmentCount = Context.SwapchainImageViews.size()/Context.FrameBufferCount;

    for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
    {
      VkFramebufferCreateInfo FBInfo{};
      FBInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

      FBInfo.attachmentCount = AttachmentCount;
      FBInfo.pAttachments = &Context.SwapchainImageViews[AttachmentCount*i];

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
  // Create the pipeline layout
  {
    VkPipelineLayoutCreateInfo PipeLayoutInfo{};
    PipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipeLayoutInfo.setLayoutCount = 0;
    PipeLayoutInfo.pSetLayouts = nullptr;
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
  /*
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
  */

  VkPipelineRasterizationStateCreateInfo RasterState{};

  // Rasterization Info
  {
    RasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterState.cullMode = VK_CULL_MODE_NONE;
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
    //PipelineCI.pDepthStencilState = &DepthState;
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
  // Create TransitBuffer
  {
    /* Define Buffer */
    VkBufferCreateInfo tBufferInfo{};
    tBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    tBufferInfo.size = 512000; // 512KB
    tBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    /* Create the buffer */
    if((Error = vkCreateBuffer(Context.Device, &tBufferInfo, nullptr, &Context.TransitBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create transfer buffer with error: " + std::to_string(Error));
    }

    /* Get memory requirements */
    VkMemoryRequirements tMemReq;
    vkGetBufferMemoryRequirements(Context.Device, Context.TransitBuffer, &tMemReq);

    /* Define allocation */
    VkMemoryAllocateInfo tAllocInfo{};
    tAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tAllocInfo.pNext = nullptr;
    tAllocInfo.allocationSize = tMemReq.size;
    tAllocInfo.memoryTypeIndex = Context.Memory.Visible;

    /* Allocate memory */
    if((Error = vkAllocateMemory(Context.Device, &tAllocInfo, nullptr, &Context.TransitMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate transit memory with error: " + std::to_string(Error));
    }

    /* Bind Buffer to allocated memory */
    if((Error = vkBindBufferMemory(Context.Device, Context.TransitBuffer, Context.TransitMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind trasnit buffer and memory with error: " + std::to_string(Error));
    }
  }

  // Repeat with vertex buffer
  {
    VkBufferCreateInfo vBufferInfo{};
    vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vBufferInfo.size = sizeof(Vertex)*3;
    vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Error = vkCreateBuffer(Context.Device, &vBufferInfo, nullptr, &Context.TriangleVertexBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create triangle vertex buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements vMemReq;
    vkGetBufferMemoryRequirements(Context.Device, Context.TriangleVertexBuffer, &vMemReq);

    VkMemoryAllocateInfo vAllocInfo{};
    vAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vAllocInfo.pNext = nullptr;
    vAllocInfo.allocationSize = vMemReq.size;
    vAllocInfo.memoryTypeIndex = Context.Memory.Local;

    if(vMemReq.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) std::cout << "Vertex buffer supports Local\n";

    if((Error = vkAllocateMemory(Context.Device, &vAllocInfo, nullptr, &Context.TriangleVertexMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate Triangle Vertex memory with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(Context.Device, Context.TriangleVertexBuffer, Context.TriangleVertexMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind Triangle vertex buffer to memory with error: " + std::to_string(Error));
    }
  }

  // Repeat with Index buffer
  {
    VkBufferCreateInfo iBufferInfo{};
    iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    iBufferInfo.size = sizeof(uint32_t) * 3;
    iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    iBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Error = vkCreateBuffer(Context.Device, &iBufferInfo, nullptr, &Context.TriangleIndexBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create Triangle Index buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements iMemReq;
    vkGetBufferMemoryRequirements(Context.Device, Context.TriangleIndexBuffer, &iMemReq);

    VkMemoryAllocateInfo iAllocInfo{};
    iAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    iAllocInfo.pNext = nullptr;
    iAllocInfo.allocationSize = iMemReq.size;
    iAllocInfo.memoryTypeIndex = Context.Memory.Local;

    if(iMemReq.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) std::cout << "Index buffer supports Local\n";

    if((Error = vkAllocateMemory(Context.Device, &iAllocInfo, nullptr, &Context.TriangleIndexMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate Triangle Index Memory with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(Context.Device, Context.TriangleIndexBuffer, Context.TriangleIndexMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind Triangle index memory with error: " + std::to_string(Error));
    }
  }

  // Map Transit Memory
  void* TransitMemory;
  vkMapMemory(Context.Device, Context.TransitMemory, 0, 512000, 0, &TransitMemory);

  // Fill Transit Buffer with Vertices
  {
    memcpy((Vertex*)TransitMemory, TriangleMesh.Vertices, sizeof(Vertex)*3);
  }

  // Transfer Vertices from Transit to Vertex Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(Vertex) * 3;
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(Context.MemoryCmd, Context.TransitBuffer, Context.TriangleVertexBuffer, 1, &CopyInfo);
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
    memcpy((uint32_t*)TransitMemory, TriangleMesh.Indices, sizeof(uint32_t)*3);
  }

  // Copy Indices from Transit to Index Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(uint32_t) * 3;
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(Context.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(Context.MemoryCmd, Context.TransitBuffer, Context.TriangleIndexBuffer, 1, &CopyInfo);
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

void RecordRender()
{
  // record the render command for each frame buffer
  for(uint32_t i = 0; i < Context.FrameBufferCount; i++)
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.pNext = nullptr;

    VkClearValue ClearValue{};

    // Color Values
    ClearValue.color.int32[0] = 0; ClearValue.color.int32[1] = 0; ClearValue.color.int32[2] = 0; ClearValue.color.int32[3] = 0;

    // Depth Values
    //ClearValue.depthStencil.depth = 0.f; ClearValue.depthStencil.stencil = 0;

    VkRenderPassBeginInfo RenderPassInfo{};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.pNext = nullptr;
    RenderPassInfo.renderPass = Context.Renderpass;

    RenderPassInfo.renderArea.extent = {1280, 720};
    RenderPassInfo.renderArea.offset = {0, 0};

    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &ClearValue;

    RenderPassInfo.framebuffer = Context.FrameBuffers[i];

    vkBeginCommandBuffer(Context.RenderBuffers[i], &BeginInfo);

      vkCmdBeginRenderPass(Context.RenderBuffers[i], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkDeviceSize Offset = 0;

        // Bind the Mesh information and pipeline.
        vkCmdBindVertexBuffers(Context.RenderBuffers[i], 0, 1, &Context.TriangleVertexBuffer, &Offset);
        vkCmdBindIndexBuffer(Context.RenderBuffers[i], Context.TriangleIndexBuffer, Offset, VK_INDEX_TYPE_UINT32);
        vkCmdBindPipeline(Context.RenderBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Context.Pipeline);

        // Draw the mesh
        vkCmdDrawIndexed(Context.RenderBuffers[i], 3/*as specified in Triangle's struct definition*/, 1, 0, 0, 0);

      vkCmdEndRenderPass(Context.RenderBuffers[i]);

    vkEndCommandBuffer(Context.RenderBuffers[i]);
  }
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

void Cleanup()
{
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

  vkDestroyBuffer(Context.Device, Context.TriangleVertexBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.TriangleVertexMemory, nullptr);

  vkDestroyBuffer(Context.Device, Context.TriangleIndexBuffer, nullptr);
  vkFreeMemory(Context.Device, Context.TriangleIndexMemory, nullptr);

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

