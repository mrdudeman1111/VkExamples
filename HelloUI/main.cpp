#include "main.h"

#include <exception>

#include <GLFW/glfw3.h>

#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

VkResult Err;

struct Buffer
{
  VkBuffer Buffer;
  VkDeviceMemory Memory;
};

Buffer CreateBuffer(VkDevice& Device, uint32_t Size, VkBufferUsageFlags Usage, uint32_t MemoryIdx)
{
  Buffer Ret;

  VkBufferCreateInfo BufferCI{};
  VkMemoryRequirements BufferMemReq{};
  VkMemoryAllocateInfo AllocInfo{};

  BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  BufferCI.usage = Usage;
  BufferCI.size = Size;
  BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if((Err = vkCreateBuffer(Device, &BufferCI, nullptr, &Ret.Buffer)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create buffer with error: " + std::to_string(Err));
  }

  vkGetBufferMemoryRequirements(Device, Ret.Buffer, &BufferMemReq);

  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.allocationSize = BufferMemReq.size;
  AllocInfo.memoryTypeIndex = MemoryIdx;

  if((Err = vkAllocateMemory(Device, &AllocInfo, nullptr, &Ret.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate buffer memory with error: " + std::to_string(Err));
  }

  if((Err = vkBindBufferMemory(Device, Ret.Buffer, Ret.Memory, 0)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to bind buffer memory with error: " + std::to_string(Err));
  }

  return Ret;
}

void CreateInstance(VkInstance& Instance, std::vector<const char*>& Layers, std::vector<const char*>& Extensions)
{
  VkInstanceCreateInfo InstanceCI{};
  InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  InstanceCI.enabledLayerCount = Layers.size();
  InstanceCI.ppEnabledLayerNames = Layers.data();
  InstanceCI.enabledExtensionCount = Extensions.size();
  InstanceCI.ppEnabledExtensionNames = Extensions.data();

  if((Err = vkCreateInstance(&InstanceCI, nullptr, &Instance)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create instance with error: " + std::to_string(Err));
  }

  return;
}

void CreatePhysicalDevice(VkInstance& Instance, VkPhysicalDevice& PhysicalDevice, VkPhysicalDeviceType DesiredType)
{
  uint32_t PDeviceCount;
  vkEnumeratePhysicalDevices(Instance, &PDeviceCount, nullptr);
  std::vector<VkPhysicalDevice> PhysicalDevices(PDeviceCount);
  vkEnumeratePhysicalDevices(Instance, &PDeviceCount, PhysicalDevices.data());

  for(uint32_t i = 0; i < PDeviceCount; i++)
  {
    VkPhysicalDeviceProperties Props;
    vkGetPhysicalDeviceProperties(PhysicalDevices[i], &Props);

    if(Props.deviceType == DesiredType)
    {
      PhysicalDevice = PhysicalDevices[i];
      return;
    }
  }

  throw std::runtime_error("Failed to find physical device");
}

uint32_t GetQueueFamily(VkPhysicalDevice& PhysicalDevice, VkQueueFlags QueueType)
{
  uint32_t FamCount;
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamCount, nullptr);
  std::vector<VkQueueFamilyProperties> Families(FamCount);
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamCount, Families.data());

  for(uint32_t i = 0; i < FamCount; i++)
  {
    if(Families[i].queueFlags & QueueType)
    {
      return i;
    }
  }

  return -1;
}

void CreateDevice(VkPhysicalDevice& PhysicalDevice, VkDevice& Device, VkQueue& Queue, uint32_t GraphicsIndex, std::vector<const char*> Extensions)
{
  VkDeviceQueueCreateInfo GraphicsCI{};
  GraphicsCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  GraphicsCI.queueFamilyIndex = GraphicsIndex;
  GraphicsCI.queueCount = 1;

  VkDeviceCreateInfo DeviceCI{};
  DeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  DeviceCI.enabledExtensionCount = Extensions.size();
  DeviceCI.ppEnabledExtensionNames = Extensions.data();
  DeviceCI.queueCreateInfoCount = 1;
  DeviceCI.pQueueCreateInfos = &GraphicsCI;

  if(vkCreateDevice(PhysicalDevice, &DeviceCI, nullptr, &Device) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create device\n");
  }

  vkGetDeviceQueue(Device, GraphicsIndex, 0, &Queue);
}

VkSurfaceFormatKHR GetRenderFormat(VkPhysicalDevice& PhysicalDevice, VkSurfaceKHR& Surface)
{
  uint32_t FormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> Formats(FormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount, Formats.data());

  for(uint32_t i = 0; i < FormatCount; i++)
  {
    if(Formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
    {
      return Formats[i];
    }
  }
}

void CreateCommand(VkDevice& Device, VkQueue& Queue, VkCommandPool& Pool, VkCommandBuffer& Buffer)
{
  VkResult Err;

  VkCommandPoolCreateInfo PoolCI{};
  PoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  PoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if((Err = vkCreateCommandPool(Device, &PoolCI, nullptr, &Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create command pool with error: " + std::to_string(Err));
  }

  VkCommandBufferAllocateInfo BufferAlloc{};
  BufferAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  BufferAlloc.commandBufferCount = 1;
  BufferAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  BufferAlloc.commandPool = Pool;

  if((Err = vkAllocateCommandBuffers(Device, &BufferAlloc, &Buffer)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate command buffer with error: " + std::to_string(Err));
  }
}

void CreateSwapchain(VkDevice& Device, VkSurfaceKHR& Surface, VkSwapchainKHR& Swapchain, VkQueue& Queue, VkCommandBuffer cmdBuffer, std::vector<VkImage>& SwapchainImages, VkFormat Format, VkExtent2D Extent)
{
  VkSwapchainCreateInfoKHR SwapCI{};
  SwapCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  SwapCI.pNext = nullptr;
  SwapCI.clipped = VK_FALSE;
  SwapCI.surface = Surface;
  SwapCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  SwapCI.imageExtent = Extent;
  SwapCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  SwapCI.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  SwapCI.imageFormat = Format;
  SwapCI.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
  SwapCI.minImageCount = 3;
  SwapCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  SwapCI.imageArrayLayers = 1;
  SwapCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if((Err = vkCreateSwapchainKHR(Device, &SwapCI, nullptr, &Swapchain)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create swapchain with error: " + std::to_string(Err));
  }

  uint32_t ImgCount;
  vkGetSwapchainImagesKHR(Device, Swapchain, &ImgCount, nullptr);
  SwapchainImages.resize(ImgCount);
  vkGetSwapchainImagesKHR(Device, Swapchain, &ImgCount, SwapchainImages.data());

  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(cmdBuffer, &BeginInfo);

  VkImageMemoryBarrier Barriers[ImgCount];

  for(uint32_t i = 0; i < ImgCount; i++)
  {
    Barriers[i] = {};

    Barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barriers[i].pNext = nullptr;
    Barriers[i].image = SwapchainImages[i];
    Barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Barriers[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    Barriers[i].srcAccessMask = VK_ACCESS_NONE;
    Barriers[i].dstAccessMask = VK_ACCESS_NONE;
    Barriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barriers[i].subresourceRange.layerCount = 1;
    Barriers[i].subresourceRange.levelCount = 1;
    Barriers[i].subresourceRange.baseMipLevel = 0;
    Barriers[i].subresourceRange.baseArrayLayer = 0;
  }

  vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, ImgCount, Barriers);

  vkEndCommandBuffer(cmdBuffer);

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &cmdBuffer;

  vkQueueSubmit(Queue, 1, &SubmitInfo, VK_NULL_HANDLE);
}

void CreateRenderpass(VkDevice& Device, VkRenderPass& Renderpass, VkFormat SwapchainFormat)
{
  VkAttachmentDescription ColorAtt{};
  ColorAtt.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  ColorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  ColorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  ColorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  ColorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  ColorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  ColorAtt.format = SwapchainFormat;
  ColorAtt.samples = VK_SAMPLE_COUNT_1_BIT;

  VkAttachmentReference ColorRef{};
  ColorRef.attachment = 0;
  ColorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription Subpass{};
  Subpass.colorAttachmentCount = 1;
  Subpass.pColorAttachments = &ColorRef;

  VkRenderPassCreateInfo RenderpassCI{};
  RenderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  RenderpassCI.subpassCount = 1;
  RenderpassCI.pSubpasses = &Subpass;
  RenderpassCI.attachmentCount = 1;
  RenderpassCI.pAttachments = &ColorAtt;

  if((Err = vkCreateRenderPass(Device, &RenderpassCI, nullptr, &Renderpass)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create renderpass with error: " + std::to_string(Err));
  }
}

void CreateFramebuffers(VkDevice& Device, VkRenderPass& Renderpass, VkFormat Format, VkExtent2D Extent, std::vector<VkImage>& SwapchainImages, std::vector<VkImageView>& SwapchainImageViews, std::vector<VkFramebuffer>& FrameBuffers)
{
  FrameBuffers.resize(SwapchainImages.size());
  SwapchainImageViews.resize(SwapchainImages.size());

  for(uint32_t i = 0; i < SwapchainImages.size(); i++)
  {
    VkImageViewCreateInfo ViewCI{};
    ViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewCI.format = Format;
    ViewCI.image = SwapchainImages[i];
    ViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

    ViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ViewCI.subresourceRange.layerCount = 1;
    ViewCI.subresourceRange.levelCount = 1;
    ViewCI.subresourceRange.baseMipLevel = 0;
    ViewCI.subresourceRange.baseArrayLayer = 0;

    ViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
    ViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
    ViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
    ViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

    if((Err = vkCreateImageView(Device, &ViewCI, nullptr, &SwapchainImageViews[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create image view for swapchain image with error: " + std::to_string(Err));
    }

    VkFramebufferCreateInfo FrameBufferCI{};
    FrameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FrameBufferCI.attachmentCount = 1;
    FrameBufferCI.pAttachments = &SwapchainImageViews[i];
    FrameBufferCI.layers = 1;
    FrameBufferCI.width = Extent.width;
    FrameBufferCI.height = Extent.height;
    FrameBufferCI.renderPass = Renderpass;

    if((Err = vkCreateFramebuffer(Device, &FrameBufferCI, nullptr, &FrameBuffers[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create frame buffers with error: " + std::to_string(Err));
    }
  }
}

static inline void CreateFence(VkDevice& Device, VkFence& Fence)
{
  VkResult Err;

  VkFenceCreateInfo FenceCI{};
  FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if((Err = vkCreateFence(Device, &FenceCI, nullptr, &Fence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create fence with error: " + std::to_string(Err));
  }

  return;
}

static inline void CreateSemaphore(VkDevice& Device, VkSemaphore& Semaphore)
{
  VkResult Err;

  VkSemaphoreCreateInfo SemCI{};
  SemCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if((Err = vkCreateSemaphore(Device, &SemCI, nullptr, &Semaphore)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create semaphore with error: " + std::to_string(Err));
  }

  return;
}

std::string glfwString = "";

void KeyboardCallback(GLFWwindow* pWindow, unsigned int CodePoint)
{
  std::cout << "GLFW character pressed: " << CodePoint << " " << (char)CodePoint << '\n';
  glfwString.append((char*)&CodePoint);
}

void SpecKeyboardCallback(GLFWwindow* pWindow, int Key, int ScanCode, int Action, int Modifiers)
{
  if(Key == GLFW_KEY_BACKSPACE && Action == GLFW_PRESS && glfwString.size() > 0)
  {
    glfwString.pop_back();
  }
  else if(Key == GLFW_KEY_BACKSPACE && Action == GLFW_REPEAT && glfwString.size() > 0)
  {
    glfwString.pop_back();
  }
}


int main()
{
  std::vector<const char*> InstLayers, InstExts;
  std::vector<const char*> DevExts;

  VkInstance Instance;
  VkPhysicalDevice PhysicalDevice;

  uint32_t LocalMemory = -1;
  uint32_t HostMemory = -1;

  VkDevice Device;
  VkQueue GraphicsQueue;
  VkCommandPool RenderPool;
  VkCommandBuffer RenderBuffer;
  VkSurfaceFormatKHR RenderFormat;
  VkSurfaceKHR Surface;
  VkSwapchainKHR Swapchain;
  std::vector<VkImage> SwapchainImages;
  std::vector<VkImageView> SwapchainImageViews;
  VkRenderPass RenderPass;
  std::vector<VkFramebuffer> FrameBuffers;

  VkFence ImageFence;
  VkFence RenderFence;
  VkSemaphore RenderSemaphore;

  GLFWwindow* pWindow;


  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  pWindow = glfwCreateWindow(1280, 720, "UI Renderer", NULL, NULL);
  uint32_t glfwExtCount;
  const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

  for(uint32_t i = 0; i < glfwExtCount; i++)
  {
    InstExts.push_back(glfwExt[i]);
  }

  InstLayers.push_back("VK_LAYER_KHRONOS_validation");
  DevExts.push_back("VK_KHR_swapchain");

  CreateInstance(Instance, InstLayers, InstExts);

  CreatePhysicalDevice(Instance, PhysicalDevice, VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);



  VkPhysicalDeviceMemoryProperties MemProps;
  vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProps);

  for(uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
  {
    if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && LocalMemory == -1)
    {
      LocalMemory = i;
    }
    else if(MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && HostMemory == -1)
    {
      HostMemory = i;
    }
  }




  CreateDevice(PhysicalDevice, Device, GraphicsQueue, GetQueueFamily(PhysicalDevice, VK_QUEUE_GRAPHICS_BIT), DevExts);

  CreateCommand(Device, GraphicsQueue, RenderPool, RenderBuffer);

  glfwCreateWindowSurface(Instance, pWindow, nullptr, &Surface);

  RenderFormat = GetRenderFormat(PhysicalDevice, Surface);

  CreateSwapchain(Device, Surface, Swapchain, GraphicsQueue, RenderBuffer, SwapchainImages, RenderFormat.format, {1280, 720});

  CreateRenderpass(Device, RenderPass, RenderFormat.format);

  CreateFramebuffers(Device, RenderPass, RenderFormat.format, {1280, 720}, SwapchainImages, SwapchainImageViews, FrameBuffers);

  UI::RendererCreateInfo UICreateInfo{};
  UICreateInfo.pWindow = pWindow;
  UICreateInfo.pInstance = &Instance;
  UICreateInfo.pPhysicalDevice = &PhysicalDevice;
  UICreateInfo.pDevice = &Device;
  UICreateInfo.pRenderpass = &RenderPass;
  UICreateInfo.Subpass = 0;
  UICreateInfo.pGraphicsQueue = &GraphicsQueue;
  UICreateInfo.LocalMemoryIndex = LocalMemory;
  UICreateInfo.HostMemoryIndex = HostMemory;

  UI::vkUI.Init(UICreateInfo);
  UI::vkUI.LoadFonts();
  UI::vkUI.CreateUIPipeline();
  UI::vkUI.CreateRectPipeline();

  Widgets::Text* MainTxt = UI::vkUI.CreateTextWidget({50.f, 100.f}, {300.f, 300.f}, EK_FONT_SIZE_MEDIUM, "");
  Widgets::Box* MainBox = UI::vkUI.CreateBoxWidget({20.f, 70.f}, {360.f, 360.f}, {0.5f, 0.f, 1.f}, 0.05f);
  Widgets::Box* SecondBox = UI::vkUI.CreateBoxWidget({400.f, 200.f}, {300.f, 300.f}, {0.5f, 1.f, 0.f}, 0.05f);

  CreateFence(Device, ImageFence);
  CreateFence(Device, RenderFence);
  CreateSemaphore(Device, RenderSemaphore);

  glfwSetCharCallback(pWindow, KeyboardCallback);

  glfwSetKeyCallback(pWindow, SpecKeyboardCallback);

  uint32_t FrameBufferIndex;

  VkCommandBufferBeginInfo cmdBeginInfo{};
  cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  VkRect2D RenderRegion;
  RenderRegion.offset = {0, 0};
  RenderRegion.extent = {1280, 720};

  VkClearValue Clear{};

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = &RenderBuffer;
  SubmitInfo.signalSemaphoreCount = 1;
  SubmitInfo.pSignalSemaphores = &RenderSemaphore;

  VkPresentInfoKHR PresentInfo{};
  PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  PresentInfo.swapchainCount = 1;
  PresentInfo.pSwapchains = &Swapchain;
  PresentInfo.pImageIndices = &FrameBufferIndex;
  PresentInfo.waitSemaphoreCount = 1;
  PresentInfo.pWaitSemaphores = &RenderSemaphore;

  while(!glfwWindowShouldClose(pWindow))
  {
    glfwPollEvents();

    MainTxt->strText = glfwString;

    UI::vkUI.UpdateWidgets();

    vkAcquireNextImageKHR(Device, Swapchain, UINT32_MAX, VK_NULL_HANDLE, ImageFence, &FrameBufferIndex);

    vkWaitForFences(Device, 1, &ImageFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Device, 1, &ImageFence);

    VkRenderPassBeginInfo rpBeginInfo{};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = RenderPass;
    rpBeginInfo.renderArea = RenderRegion;
    rpBeginInfo.framebuffer = FrameBuffers[FrameBufferIndex];
    rpBeginInfo.clearValueCount = 1;
    rpBeginInfo.pClearValues = &Clear;

    vkBeginCommandBuffer(RenderBuffer, &cmdBeginInfo);
      vkCmdBeginRenderPass(RenderBuffer, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        UI::vkUI.DrawRect(RenderBuffer);
        UI::vkUI.DrawUI(RenderBuffer);
      vkCmdEndRenderPass(RenderBuffer);
    vkEndCommandBuffer(RenderBuffer);

    vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, RenderFence);

    vkQueuePresentKHR(GraphicsQueue, &PresentInfo);

    vkWaitForFences(Device, 1, &RenderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Device, 1, &RenderFence);
  }

  std::cout << "Clean run\n";

  return 0;
}
