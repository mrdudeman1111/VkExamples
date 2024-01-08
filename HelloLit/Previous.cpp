#include <main.h>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>

#include <sail/sail.h>
#include <sail-common/sail-common.h>
#include <sail-manip/sail-manip.h>

VkResult Error;
Context VkContext;
Camera Cam;
Mesh PrimaryMesh;
Constants PushConstants;

#ifndef NDEBUG
  std::vector<const char*> InstLayers = {"VK_LAYER_KHRONOS_validation"}; // VK_LAYER_KHRONOS_VALIDATION so we can get debug info in CLI
#else
  std::vector<const char*> InstLayers = {};
#endif
std::vector<const char*> InstExt = {"VK_KHR_surface"}; // VK_KHR_surface so we can interface with the surface, surface is a fancy image used for presentation to a display in this case.
std::vector<const char*> DevLayers = {};
std::vector<const char*> DevExt = {"VK_KHR_swapchain"}; // VK_KHR_swapchain so that we can create and use Swapchain objects to handle presentation to screen.

Camera::Camera()
{
  Position = glm::vec3(0.f, 0.f, 5.f);
  Rotation = glm::vec3(0.f, 0.f, 0.f);

  Matrices.World = glm::mat4(1.f);

  Matrices.Proj = glm::perspective(glm::radians(45.f), 16.f/9.f, 0.1f, 100.f);
  Matrices.Proj[1][1] *= -1;

  Matrices.View = glm::inverse(glm::translate(glm::mat4(1.f), Position));

  CameraMatrix = glm::mat4(1.f);
}

void Camera::Update()
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
    glfwGetCursorPos(VkContext.Window, &X, &Y);

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
    if(glfwGetKey(VkContext.Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
      MovementModifier = 2.f;
    }
    else
    {
      MovementModifier = 1.f;
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_W) == GLFW_PRESS)
    {
      Position += (ForwardVector*MoveSpeed*MovementModifier);
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_S) == GLFW_PRESS)
    {
      Position += (ForwardVector*MoveSpeed*MovementModifier*-1.f);
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_D) == GLFW_PRESS)
    {
      Position += (RightVector*MoveSpeed*MovementModifier);
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_A) == GLFW_PRESS)
    {
      Position += (RightVector*MoveSpeed*MovementModifier*-1.f);
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_E) == GLFW_PRESS)
    {
      Position += (UpVector*MoveSpeed*MovementModifier);
    }
    if(glfwGetKey(VkContext.Window, GLFW_KEY_Q) == GLFW_PRESS)
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

  if(glfwGetKey(VkContext.Window, GLFW_KEY_TAB) == GLFW_PRESS)
  {
    if(WindowFocused)
    {
      WindowFocused = false;
      glfwSetInputMode(VkContext.Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
      WindowFocused = true;
      glfwSetInputMode(VkContext.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }
}

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
  std::vector<VkVertexInputAttributeDescription> Ret(4);

  Ret[0].binding = 0;
  Ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[0].location = 0;
  Ret[0].offset = offsetof(Vertex, vPos);

  Ret[1].binding = 0;
  Ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[1].location = 1;
  Ret[1].offset = offsetof(Vertex, vColor);

  Ret[2].binding = 0;
  Ret[2].format = VK_FORMAT_R32G32B32_SFLOAT;
  Ret[2].location = 2;
  Ret[2].offset = offsetof(Vertex, vNorm);

  Ret[3].binding = 0;
  Ret[3].format = VK_FORMAT_R32G32_SFLOAT;
  Ret[3].location = 3;
  Ret[3].offset = offsetof(Vertex, vCoord);

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

  if((Error = vkCreateImage(VkContext.Device, &ImageCI, nullptr, Image)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image with error: " + std::to_string(Error));
  }

  VkMemoryRequirements MemReq;
  vkGetImageMemoryRequirements(VkContext.Device, *Image, &MemReq);

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.allocationSize = MemReq.size;
  AllocInfo.memoryTypeIndex = VkContext.Memory.Local;


  if((Error = vkAllocateMemory(VkContext.Device, &AllocInfo, nullptr, Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate image error: " + std::to_string(Error));
  }

  if((Error = vkBindImageMemory(VkContext.Device, *Image, *Memory, 0)) != VK_SUCCESS)
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

  if((vkCreateImageView(VkContext.Device, &ViewCI, nullptr, View)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image view with error: " + std::to_string(Error));
  }
}

void CopyBuffer(VkBuffer* Src, VkBuffer* Dst, VkBufferCopy* CopyInfo)
{
  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
    vkCmdCopyBuffer(VkContext.MemoryCmd, *Src, *Dst, 1, CopyInfo);
  vkEndCommandBuffer(VkContext.MemoryCmd);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

  if((Error = vkQueueSubmit(VkContext.GraphicsQueue,  1, &SubmitInfo, VkContext.MemoryFence)) == VK_SUCCESS)
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

  vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
    vkCmdCopyBufferToImage(VkContext.MemoryCmd, *Src, *Dst, Layout, 1, &CopyInfo);
  vkEndCommandBuffer(VkContext.MemoryCmd);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

  if((Error = vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.MemoryFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit Buffer to Image copy with error: " + std::to_string(Error));
  }
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

  if((Error = vkCreateInstance(&InstCI, nullptr, &VkContext.Instance)) != VK_SUCCESS)
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
    vkEnumeratePhysicalDevices(VkContext.Instance, &PhysicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> Devices(PhysicalDeviceCount);
    vkEnumeratePhysicalDevices(VkContext.Instance, &PhysicalDeviceCount, Devices.data());

    // Enumerate over list until we find GPU.
    for(uint32_t i = 0; i < PhysicalDeviceCount; i++)
    {
      VkPhysicalDeviceProperties DevProps;
      vkGetPhysicalDeviceProperties(Devices[i], &DevProps);

      if(DevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        VkContext.PhysicalDevice = Devices[i];
      }
    }
  }

  // Find all the Queue Families. You might want to read up on this real quick
  {
    // Retrieve list of queue families.
    uint32_t QFamPropCount;
    vkGetPhysicalDeviceQueueFamilyProperties(VkContext.PhysicalDevice, &QFamPropCount, nullptr);
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QFamPropCount);
    vkGetPhysicalDeviceQueueFamilyProperties(VkContext.PhysicalDevice, &QFamPropCount, QueueFamilyProperties.data());

    bool Graphics = false;
    bool Compute = false;

    // Enumerate over queue families until we find a Graphics family and a Compute family. (sometimes there are several)
    for(uint32_t i = 0; i < QFamPropCount; i++)
    {
      if(QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !Graphics)
      {
        VkContext.Families.Graphics = i;
      }
      else if(QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !Compute)
      {
        VkContext.Families.Compute = i;
      }
    }
  }

  // Find The Host visible and device local memory
  {
    VkPhysicalDeviceMemoryProperties MemProps;

    vkGetPhysicalDeviceMemoryProperties(VkContext.PhysicalDevice, &MemProps);

    bool Local = false, Visible = false;

    for(uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
    {
      if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && !Local)
      {
        VkContext.Memory.Local = i;
        Local = true;
        std::cout << "Local Properties " << MemProps.memoryTypes[i].propertyFlags << '\n';
      }
      else if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && !Visible)
      {
        VkContext.Memory.Visible = i;
        Visible = true;
        std::cout << "Visibile properties " << MemProps.memoryTypes[i].propertyFlags << '\n';
      }
    }

    assert(VkContext.Memory.Local != -1);
    assert(VkContext.Memory.Visible != -1);
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
    QInfo[0].queueFamilyIndex = VkContext.Families.Graphics;
    QInfo[1].queueFamilyIndex = VkContext.Families.Compute;
  }

  // Create Device and queues
  {
    VkDeviceCreateInfo DeviceInfo{};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceInfo.enabledExtensionCount = DevExt.size();
    DeviceInfo.ppEnabledExtensionNames = DevExt.data();
    DeviceInfo.queueCreateInfoCount = 2;
    DeviceInfo.pQueueCreateInfos = QInfo;

    if((Error = vkCreateDevice(VkContext.PhysicalDevice, &DeviceInfo, nullptr, &VkContext.Device)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to Create Device " + std::to_string(Error));
    }
  }

  // Grab the returned Queues.
  {
    vkGetDeviceQueue(VkContext.Device, VkContext.Families.Graphics, 0, &VkContext.GraphicsQueue);
    vkGetDeviceQueue(VkContext.Device, VkContext.Families.Compute, 0, &VkContext.ComputeQueue);
  }
}

void InitSwapchain(VkExtent2D WindowExtent)
{
  VkContext.WindowExtent = WindowExtent;

  // Create a Window on the desktop, and a surface to send rendered images to.
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    VkContext.Window = glfwCreateWindow(WindowExtent.width, WindowExtent.height, "Hello Triangle", NULL, NULL);

    if((Error = glfwCreateWindowSurface(VkContext.Instance, VkContext.Window, nullptr, &VkContext.Surface)) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create window surface with error: " + std::to_string(Error));
    }
  }

  VkSurfaceCapabilitiesKHR SurfaceCapabilities;

  // Get Surface Format, and colorspace.
  {
    // We get the display's Swapchain Capabilities through the GPU.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkContext.PhysicalDevice, VkContext.Surface, &SurfaceCapabilities);

    // The Number of framebuffers we can have is dependent on the surface's capabilities.
    // so we will use MinimumFramebufferCount+1 (The +1 is good practice, you can exclude it, but it might result in instability and tearing)
    VkContext.FrameBufferCount = SurfaceCapabilities.minImageCount + 1;

    // Get Monitor Format
    {
      // Get display's available formats, again, through the GPU.
      uint32_t SurfaceFormatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext.PhysicalDevice, VkContext.Surface, &SurfaceFormatCount, nullptr);
      std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(VkContext.PhysicalDevice, VkContext.Surface, &SurfaceFormatCount, SurfaceFormats.data());

      // Enumerate over available surface formats until we find compatible format
      for(uint32_t i = 0; i < SurfaceFormatCount; i++)
      {
        // We want RGB output, just cuz. We could look for other formats, but then we would need to adjust some formulas, and I don't want to do that.
        if(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
          VkContext.SurfaceFormat = SurfaceFormats[i];
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
    SwapchainCI.imageFormat = VkContext.SurfaceFormat.format;
    SwapchainCI.imageColorSpace = VkContext.SurfaceFormat.colorSpace;
    SwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainCI.imageArrayLayers = 1;

    SwapchainCI.clipped = VK_FALSE;
    SwapchainCI.surface = VkContext.Surface;
    SwapchainCI.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    SwapchainCI.minImageCount = VkContext.FrameBufferCount;

    SwapchainCI.preTransform = SurfaceCapabilities.currentTransform;

    // what do we do when the window on the desktop, is placed over another window.
    // In this case, we want to occlude the window behind ours. So we will use Opaque;
    SwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    if(vkCreateSwapchainKHR(VkContext.Device, &SwapchainCI, nullptr, &VkContext.Swapchain) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create swapchain");
    }
  }

  // Grab Swapchain Images for later.
  {
    uint32_t ImageCount;
    vkGetSwapchainImagesKHR(VkContext.Device, VkContext.Swapchain, &ImageCount, nullptr);

    // sometimes, the swapchain will make more images than it needs, for safety
    VkContext.FrameBufferCount = ImageCount;
    VkContext.SwapchainImages.resize(ImageCount);

    vkGetSwapchainImagesKHR(VkContext.Device, VkContext.Swapchain, &ImageCount, VkContext.SwapchainImages.data());
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
    ColorDescription.format = VkContext.SurfaceFormat.format;
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
    // this is the first attachment in the framebuffer, so it's at index 0
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

    VkSubpassDependency DepthDep{};

    {
      DepthDep.srcSubpass = VK_SUBPASS_EXTERNAL;
      DepthDep.dstSubpass = 0;

      DepthDep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      DepthDep.srcAccessMask = 0;

      DepthDep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      DepthDep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkRenderPassCreateInfo RPInfo{};

    RPInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    RPInfo.dependencyCount = 1;
    RPInfo.pDependencies = &DepthDep;

    RPInfo.subpassCount = 1;
    RPInfo.pSubpasses = &SubpassDescription;

    RPInfo.attachmentCount = 2;
    RPInfo.pAttachments = Descriptions;

    if((Error = vkCreateRenderPass(VkContext.Device, &RPInfo, nullptr, &VkContext.Renderpass)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create renderpass with error: " + std::to_string(Error));
    }
  }
}

void InitFrameBuffer()
{
  // Create Swapchain Image views
  {
    VkContext.SwapchainImageViews.resize(VkContext.SwapchainImages.size());

    VkContext.DepthImages.resize(VkContext.SwapchainImages.size());
    VkContext.DepthImageViews.resize(VkContext.SwapchainImages.size());
    VkContext.DepthMemory.resize(VkContext.SwapchainImageViews.size());

    // Create the Image view for our Swapchain Image
    for(uint32_t i = 0; i < VkContext.FrameBufferCount; i++)
    {
      CreateImage(&VkContext.DepthImages[i], &VkContext.DepthMemory[i], VkContext.WindowExtent, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

      CreateImageView(&VkContext.DepthImages[i], &VkContext.DepthImageViews[i], VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

      VkImageViewCreateInfo ViewInfo{};
      ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      ViewInfo.format = VkContext.SurfaceFormat.format;
      ViewInfo.image = VkContext.SwapchainImages[i];
      ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

      ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      ViewInfo.subresourceRange.layerCount = 1;
      ViewInfo.subresourceRange.levelCount = 1;
      ViewInfo.subresourceRange.baseMipLevel = 0;
      ViewInfo.subresourceRange.baseArrayLayer = 0;

      ViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
      ViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
      ViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;

      if((Error = vkCreateImageView(VkContext.Device, &ViewInfo, nullptr, &VkContext.SwapchainImageViews[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create image view with error " + std::to_string(Error));
      }
    }
  }

  // Create FrameBuffers.
  {
    // Resize the Frame buffer array in preperation for creation.
    VkContext.FrameBuffers.resize(VkContext.FrameBufferCount);

    for(uint32_t i = 0; i < VkContext.FrameBufferCount; i++)
    {
      VkImageView ImageViews[] = {VkContext.SwapchainImageViews[i], VkContext.DepthImageViews[i]};

      VkFramebufferCreateInfo FBInfo{};
      FBInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

      FBInfo.attachmentCount = 2;
      FBInfo.pAttachments = ImageViews;

      FBInfo.renderPass = VkContext.Renderpass;

      FBInfo.width = VkContext.WindowExtent.width;
      FBInfo.height = VkContext.WindowExtent.height;

      FBInfo.layers = 1;

      if((Error = vkCreateFramebuffer(VkContext.Device, &FBInfo, nullptr, &VkContext.FrameBuffers[i])) != VK_SUCCESS)
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
  PoolCreateInfo.queueFamilyIndex = VkContext.Families.Graphics;

  if(vkCreateCommandPool(VkContext.Device, &PoolCreateInfo, nullptr, &VkContext.GraphicsPool) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create graphics pool with error: " + std::to_string(Error));
  }
}

void InitCommandBuffer()
{
  VkContext.RenderBuffers.resize(VkContext.FrameBufferCount);

  // allocate the command buffers
  {
    for(uint32_t i = 0; i < VkContext.FrameBufferCount; i++)
    {
      VkCommandBufferAllocateInfo RenderBufferInfo{};
      RenderBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      RenderBufferInfo.commandPool = VkContext.GraphicsPool;
      RenderBufferInfo.commandBufferCount = 1;
      RenderBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

      if((Error = vkAllocateCommandBuffers(VkContext.Device, &RenderBufferInfo, &VkContext.RenderBuffers[i])) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate command buffer " + std::to_string(i) + " with error: " + std::to_string(Error));
      }
    }

    VkCommandBufferAllocateInfo MemoryCmdInfo{};
    MemoryCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    MemoryCmdInfo.commandPool = VkContext.GraphicsPool;
    MemoryCmdInfo.commandBufferCount = 1;;
    MemoryCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if((Error = vkAllocateCommandBuffers(VkContext.Device, &MemoryCmdInfo, &VkContext.MemoryCmd)) != VK_SUCCESS)
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

    if((Error = vkCreateShaderModule(VkContext.Device, &VertexCreateInfo, nullptr, &VkContext.VertexShader)) != VK_SUCCESS)
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

    if((Error = vkCreateShaderModule(VkContext.Device, &FragmentCreateInfo, nullptr, &VkContext.FragmentShader)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create fragment shader with error: " + std::to_string(Error));
    }
  }
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

  if((Error = vkCreateDescriptorPool(VkContext.Device, &PoolInfo, nullptr, &VkContext.DescPool)) != VK_SUCCESS)
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
    AllocInfo.descriptorPool = VkContext.DescPool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &VkContext.DescLayout;

    if((Error = vkAllocateDescriptorSets(VkContext.Device, &AllocInfo, &VkContext.DescSet)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor sets with error: " + std::to_string(Error));
    }
  }

  // allocate Sampler descriptor
  {
    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = VkContext.DescPool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &VkContext.SamplerLayout;

    if((Error = vkAllocateDescriptorSets(VkContext.Device, &AllocInfo, &PrimaryMesh.TextureDescriptor)) != VK_SUCCESS)
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

    if((Error = vkCreateBuffer(VkContext.Device, &BufferCI, nullptr, &VkContext.mvpBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create MVP Buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements BufferMemoryRequirements;
    vkGetBufferMemoryRequirements(VkContext.Device, VkContext.mvpBuffer, &BufferMemoryRequirements);

    VkMemoryAllocateInfo MemAlloc{};
    MemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemAlloc.allocationSize = BufferMemoryRequirements.size;
    MemAlloc.memoryTypeIndex = VkContext.Memory.Local;

    if((Error = vkAllocateMemory(VkContext.Device, &MemAlloc, nullptr, &VkContext.mvpMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate memory with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(VkContext.Device, VkContext.mvpBuffer, VkContext.mvpMemory, 0)) != VK_SUCCESS)
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

  if((Error = vkCreateSampler(VkContext.Device, &SamplerCI, nullptr, &PrimaryMesh.Sampler_Albedo)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create mesh sampler with error: " + std::to_string(Error));
  }

  // Quickly Perform layout transition
  {
    VkImageMemoryBarrier ImgBarrier{};
    ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImgBarrier.image = PrimaryMesh.Texture_Albedo;
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

      vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
        vkCmdPipelineBarrier(VkContext.MemoryCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImgBarrier);
      vkEndCommandBuffer(VkContext.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

    vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.MemoryFence);

    vkWaitForFences(VkContext.Device, 1, &VkContext.MemoryFence, VK_TRUE, UINT64_MAX);
    vkResetFences(VkContext.Device, 1, &VkContext.MemoryFence);
  }

  // Update sampler Texture
  {
    VkDescriptorImageInfo ImageWrite{};
    ImageWrite.sampler = PrimaryMesh.Sampler_Albedo; // Target sampler
    ImageWrite.imageView = PrimaryMesh.View_Albedo;
    ImageWrite.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet WriteInfo{};
    WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    WriteInfo.dstSet = PrimaryMesh.TextureDescriptor;
    WriteInfo.pImageInfo = &ImageWrite;

    WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteInfo.descriptorCount = 1;
    WriteInfo.dstArrayElement = 0;

    vkUpdateDescriptorSets(VkContext.Device, 1, &WriteInfo, 0, nullptr);
  }
}

void InitSync()
{
  VkSemaphoreCreateInfo SemInfo{};
  SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if((Error = vkCreateSemaphore(VkContext.Device, &SemInfo, nullptr, &VkContext.RenderSemaphore)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Rendering Semaphore with error: " + std::to_string(Error));
  }

  VkFenceCreateInfo FenceInfo{};
  FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if((Error = vkCreateFence(VkContext.Device, &FenceInfo, nullptr, &VkContext.MemoryFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create memory fence with error: " + std::to_string(Error));
  }

  if((Error = vkCreateFence(VkContext.Device, &FenceInfo, nullptr, &VkContext.ImageFence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image fence with error: " + std::to_string(Error));
  }

  FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkContext.CmdFence.resize(VkContext.FrameBufferCount);

  for(uint32_t i = 0; i < VkContext.FrameBufferCount; i++)
  {
    if((Error = vkCreateFence(VkContext.Device, &FenceInfo, nullptr, &VkContext.CmdFence[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create fence with error: " + std::to_string(Error));
    }
  }
}

void SubmitRender(uint32_t RenderIndex)
{
  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &VkContext.RenderBuffers[RenderIndex];

  // this semaphore will be used during presentation to ensure that the image has been completely finished before beginning rendering again
  SubmitInfo.signalSemaphoreCount = 1;
  SubmitInfo.pSignalSemaphores = &VkContext.RenderSemaphore;

  vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.CmdFence[RenderIndex]);
}

void Present(uint32_t RenderIndex)
{
  VkPresentInfoKHR PresentInfo{};
  PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  PresentInfo.waitSemaphoreCount = 1;
  PresentInfo.pWaitSemaphores = &VkContext.RenderSemaphore;

  PresentInfo.pResults = &Error;

  PresentInfo.pImageIndices = &RenderIndex;

  PresentInfo.swapchainCount = 1;
  PresentInfo.pSwapchains = &VkContext.Swapchain;

  vkQueuePresentKHR(VkContext.GraphicsQueue, &PresentInfo);
}

