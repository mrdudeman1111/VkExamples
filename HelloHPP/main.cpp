#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

int main()
{
  std::vector<const char*> Layers;
  std::vector<const char*> InstExt;
  Layers.push_back("VK_LAYER_KHRONOS_validation");

  std::vector<const char*> DevExt;
  DevExt.push_back("VK_KHR_EXT_swapchain");

  vk::Instance Instance;
  {
    uint32_t glfwExtCount;

    vk::InstanceCreateInfo InstanceCI{};
    InstanceCI.setEnabledLayerCount(Layers.size());
    InstanceCI.setPpEnabledLayerNames(Layers.data());

    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    InstExt.resize(glfwExtCount);
    for(uint32_t i = 0; i < glfwExtCount; i++)
    {
      InstExt.push_back(glfwExts[i]);
    }

    InstanceCI.setEnabledExtensionCount(InstExt.size());
    InstanceCI.setPpEnabledExtensionNames(InstExt.data());

    Instance = vk::createInstance(InstanceCI);
  }

  vk::PhysicalDevice PhysicalDevice;
  uint32_t LocalMemory;
  uint32_t HostMemory;
  uint32_t GraphicsIndex;
  uint32_t ComputeIndex;
  {
    std::vector<vk::PhysicalDevice> PhysicalDevices;

    PhysicalDevices = Instance.enumeratePhysicalDevices();

    for(uint32_t i = 0; i < PhysicalDevices.size(); i++)
    {
      vk::PhysicalDeviceProperties PDevProperties;

      PDevProperties = PhysicalDevices[i].getProperties();
      if(PDevProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
      {
        PhysicalDevice = PhysicalDevices[i];
        break;
      }
    }

    vk::PhysicalDeviceMemoryProperties MemProps;
    MemProps = PhysicalDevice.getMemoryProperties();

    for(uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
    {
      if(MemProps.memoryTypes[i].propertyFlags == vk::MemoryPropertyFlagBits::eHostVisible)
      {
        HostMemory = i;
      }
      else if(MemProps.memoryTypes[i].propertyFlags == vk::MemoryPropertyFlagBits::eDeviceLocal)
      {
        LocalMemory = i;
      }
    }

    std::vector<vk::QueueFamilyProperties> QueueProps;
    QueueProps = PhysicalDevice.getQueueFamilyProperties();

    for(uint32_t i = 0; i < QueueProps.size(); i++)
    {
      if(QueueProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
      {
        GraphicsIndex = i;
      }
      else if(QueueProps[i].queueFlags & vk::QueueFlagBits::eCompute)
      {
        ComputeIndex = i;
      }
    }
  }

  vk::Device Device;
  vk::Queue GraphicsQueue;
  vk::Queue ComputeQueue;
  {
    std::vector<vk::DeviceQueueCreateInfo> QueueInfos;
    vk::DeviceCreateInfo DeviceCI{};

    QueueInfos.push_back({});
    QueueInfos[0].setQueueCount(1);
    QueueInfos[0].setQueueFamilyIndex(GraphicsIndex);

    QueueInfos.push_back({});
    QueueInfos[1].setQueueCount(1);
    QueueInfos[1].setQueueFamilyIndex(ComputeIndex);

    DeviceCI.setQueueCreateInfoCount(2);
    DeviceCI.setPQueueCreateInfos(QueueInfos.data());
    DeviceCI.setEnabledExtensionCount(DevExt.size());
    DeviceCI.setPpEnabledExtensionNames(DevExt.data());

    Device = PhysicalDevice.createDevice(DeviceCI);

    GraphicsQueue = Device.getQueue(GraphicsIndex, 0);
    ComputeQueue = Device.getQueue(ComputeIndex, 0);
  }

  GLFWwindow* Window;
  vk::SurfaceKHR Surface;
  {
    glfwCreateWindow(1280, 720, "Vulkan renderer", nullptr, nullptr);

    VkSurfaceKHR vkSurface;
    glfwCreateWindowSurface(static_cast<VkInstance>(Instance), Window, nullptr, &vkSurface);

    Surface = vk::SurfaceKHR(vkSurface);
  }

  vk::CommandPool cmdPool;
  {
    vk::CommandPoolCreateInfo cmdPoolCI{};
    cmdPoolCI.setQueueFamilyIndex(GraphicsIndex);

    cmdPool = Device.createCommandPool(cmdPoolCI);
  }

  vk::SurfaceFormatKHR SurfaceFormat;
  {
    std::vector<vk::SurfaceFormatKHR> SurfaceFormats;
    SurfaceFormats = PhysicalDevice.getSurfaceFormatsKHR();

    for(uint32_t i = 0; i < SurfaceFormats.size(); i++)
    {
      if(SurfaceFormats[i].colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      {
        SurfaceFormat = SurfaceFormats[i];
      }
    }
  }

  std::vector<vk::Image> SwapchainImages;

  vk::SwapchainKHR Swapchain;
  vk::Extent2D SwapchainSize{1280, 720};
  {
    vk::SurfaceCapabilitiesKHR SurfCap;
    SurfCap = PhysicalDevice.getSurfaceCapabilitiesKHR(Surface);

    vk::SwapchainCreateInfoKHR SwapchainCI{};
    SwapchainCI.setSurface(Surface);
    SwapchainCI.setClipped(VK_FALSE);

    SwapchainCI.setImageFormat(SurfaceFormat.format);
    SwapchainCI.setImageColorSpace(SurfaceFormat.colorSpace);
    SwapchainCI.setImageExtent(SwapchainSize);
    SwapchainCI.setImageSharingMode(vk::SharingMode::eExclusive);

    SwapchainCI.setMinImageCount(SurfCap.minImageCount);
    SwapchainCI.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
    SwapchainCI.setPresentMode(vk::PresentModeKHR::eMailbox);
    SwapchainCI.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

    Swapchain = Device.createSwapchainKHR(SwapchainCI);

    SwapchainImages = Device.getSwapchainImagesKHR(Swapchain);
  }

  vk::RenderPass Renderpass;
  {
    vk::AttachmentDescription outDesc;
    outDesc.setFormat(SurfaceFormat.format);

    outDesc.setStoreOp(vk::AttachmentStoreOp::eStore);
    outDesc.setLoadOp(vk::AttachmentLoadOp::eLoad);
    outDesc.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
    outDesc.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);

    outDesc.setInitialLayout(vk::ImageLayout::ePresentSrcKHR);
    outDesc.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference outRef;
    outRef.setAttachment(0);
    outRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription MainPass;
    MainPass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    MainPass.setColorAttachmentCount(1);
    MainPass.setPColorAttachments(&outRef);

    vk::RenderPassCreateInfo RenderpassCI;
  }

  std::vector<vk::ImageView> ColorViews(SwapchainImages.size());
  std::vector<vk::Framebuffer> Framebuffers(SwapchainImages.size());
  {
    for(uint32_t i = 0; i < Framebuffers.size(); i++)
    {
      vk::ImageViewCreateInfo ColorViewCI{};
      ColorViewCI.setImage(SwapchainImages[i]);
      ColorViewCI.setFormat(SurfaceFormat.format);
      ColorViewCI.setViewType(vk::ImageViewType::e2D);
      ColorViewCI.setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA));
      ColorViewCI.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

      ColorViews[i] = Device.createImageView(ColorViewCI);
    }

    for(uint32_t i = 0; i < Framebuffers.size(); i++)
    {
      vk::FramebufferCreateInfo FramebufferCI{};
      FramebufferCI.setWidth(SwapchainSize.width);
      FramebufferCI.setHeight(SwapchainSize.height);
      FramebufferCI.setLayers(1);
      FramebufferCI.setRenderPass(Renderpass);
      FramebufferCI.setAttachmentCount(1);
      FramebufferCI.setPAttachments(&ColorViews[i]);

      Framebuffers[i] = Device.createFramebuffer(FramebufferCI);
    }
  }

  vk::ShaderModule VertexShader;
  vk::ShaderModule FragmentShader;
  {
    vk::ShaderModuleCreateInfo VertexCI{};
    vk::ShaderModuleCreateInfo FragmentCI{};
    std::ifstream VertexFile;
    std::ifstream FragmentFile;
    uint32_t VertexSize;
    uint32_t FragmentSize;
    std::vector<char> VertexCode;
    std::vector<char> FragmentCode;

    // read shader binaries
    {
      VertexFile.open("Vert.spv", std::ios::binary);
      FragmentFile.open("Frag.spv", std::ios::binary);

      VertexFile.seekg(std::ios::end);
      FragmentFile.seekg(std::ios::end);

      VertexSize = VertexFile.tellg();
      FragmentSize = FragmentFile.tellg();

      VertexFile.seekg(std::ios::beg);
      FragmentFile.seekg(std::ios::beg);

      VertexFile.read(VertexCode.data(), VertexSize);
      FragmentFile.read(FragmentCode.data(), FragmentSize);
    }

    // create Shaders
    {
      VertexCI.setCodeSize(VertexSize);
      VertexCI.setPCode((const uint32_t*)VertexCode.data());

      FragmentCI.setCodeSize(FragmentSize);
      FragmentCI.setPCode((const uint32_t*)FragmentCode.data());

      VertexShader = Device.createShaderModule(VertexCI);
      FragmentShader = Device.createShaderModule(FragmentCI);
    }
  }

  vk::Pipeline TrianglePipeline;
  {
    vk::PipelineLayoutCreateInfo PipelineLayoutCI;
    PipelineLayoutCI.setSetLayoutCount(0);
    PipelineLayoutCI.setPushConstantRangeCount(0);

    vk::PipelineLayout PipelineLayout = Device.createPipelineLayout(PipelineLayoutCI);

    vk::PipelineViewportStateCreateInfo ViewportCI{};
    {
      vk::Rect2D View{};
      View.setExtent({SwapchainSize.width, SwapchainSize.height});
      View.setOffset({0, 0});

      vk::Viewport Viewport{};
      Viewport.setX(0.f);
      Viewport.setY(0.f);
      Viewport.setWidth(SwapchainSize.width);
      Viewport.setHeight(SwapchainSize.height);
      Viewport.setMinDepth(0.01f);
      Viewport.setMaxDepth(1000.f);

      ViewportCI.setScissorCount(1);
      ViewportCI.setPScissors(&View);
      ViewportCI.setViewportCount(1);
      ViewportCI.setPViewports(&Viewport);
    }

    vk::PipelineMultisampleStateCreateInfo MultiSample{};
    {
      MultiSample.setMinSampleShading(0.f);
      MultiSample.setAlphaToOneEnable(vk::True);
      MultiSample.setAlphaToCoverageEnable(vk::True);
      MultiSample.setRasterizationSamples(vk::SampleCountFlagBits::e1);
      MultiSample.setSampleShadingEnable(vk::True);
    }

    vk::PipelineColorBlendStateCreateInfo ColorBlend{};
    {
      vk::PipelineColorBlendAttachmentState ColorAtt{};
      ColorAtt.setBlendEnable(vk::False);
      ColorAtt.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

      ColorBlend.setLogicOpEnable(vk::False);
      ColorBlend.setAttachmentCount(1);
      ColorBlend.setPAttachments(&ColorAtt);
    }

    vk::PipelineRasterizationStateCreateInfo Raster{};
    {
      Raster.setCullMode(vk::CullModeFlagBits::eBack);
      Raster.setFrontFace(vk::FrontFace::eCounterClockwise);
      Raster.setDepthBiasClamp(0.f);
      Raster.setDepthClampEnable(vk::False);
      Raster.setPolygonMode(vk::PolygonMode::eFill);
      Raster.setRasterizerDiscardEnable(vk::False);
    }

    vk::PipelineVertexInputStateCreateInfo VertexCI{};
    {
      VertexCI.setVertexBindingDescriptionCount(0);
      VertexCI.setVertexAttributeDescriptionCount(0);
    }

    vk::PipelineInputAssemblyStateCreateInfo InAssembly{};
    {
      InAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
      InAssembly.setPrimitiveRestartEnable(vk::False);
    }

    vk::PipelineShaderStageCreateInfo ShaderStages[2];
    {
      ShaderStages[0].setPName("main");
      ShaderStages[0].setModule(VertexShader);
      ShaderStages[0].setStage(vk::ShaderStageFlagBits::eVertex);

      ShaderStages[1].setPName("main");
      ShaderStages[1].setModule(FragmentShader);
      ShaderStages[1].setStage(vk::ShaderStageFlagBits::eFragment);
    }

    vk::GraphicsPipelineCreateInfo PipelineCI{};
    PipelineCI.setLayout(PipelineLayout);
    PipelineCI.setRenderPass(Renderpass);
    PipelineCI.setSubpass(0);
    PipelineCI.setStageCount(2);
    PipelineCI.setPStages(ShaderStages);
    PipelineCI.setPViewportState(&ViewportCI);
    PipelineCI.setPRasterizationState(&Raster);
    PipelineCI.setPMultisampleState(&MultiSample);
    PipelineCI.setPColorBlendState(&ColorBlend);
    PipelineCI.setPVertexInputState(&VertexCI);
    PipelineCI.setPInputAssemblyState(&InAssembly);

    vk::ResultValue<vk::Pipeline> Res = Device.createGraphicsPipeline(VK_NULL_HANDLE, PipelineCI);

    if(Res.result != vk::Result::eSuccess)
    {
      throw std::runtime_error("failed to create graphics pipeline");
    }

    TrianglePipeline = Res.value;
  }

  std::cout << "\nclean run\n";
}

