#include "UI.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <cwchar>
#include <vulkan/vulkan_core.h>

bool Widget::isCollide(glm::vec2 Pos)
{
  uint32_t xMin;
  uint32_t xMax;

  uint32_t yMin;
  uint32_t yMax;

  xMin = Pos.x;
  xMax = Pos.x+Width;

  yMin = Pos.y;
  yMax = Pos.y+Height;

  if(Pos.x > xMin && Pos.x < xMax)
  {
    if(Pos.y > yMin && Pos.y < yMax)
    {
      return false;
    }
  }

  return false;
}

void UIRenderer::Init(VkInstance& Instance, VkPhysicalDevice& PhysicalDevice, VkDevice& Device, VkRenderPass& RenderPass, uint32_t uiSubpass)
{
  pInstance = &Instance;
  pPhysicalDevice = &PhysicalDevice;
  pDevice = &Device;
  pRenderpass = &RenderPass;
  Subpass = uiSubpass;
}

void UIRenderer::SetOutput(GLFWwindow* pWindow, VkSurfaceKHR& Surface, VkSwapchainKHR& Swapchain)
{
  pSwapchain = &Swapchain;

  Window = pWindow;

  int Width, Height;
  glfwGetWindowSize(pWindow, &Width, &Height);
  WindowSize = { (uint32_t)Width, (uint32_t)Height };

  VkSurfaceCapabilitiesKHR SurfCap;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pPhysicalDevice, Surface, &SurfCap);
  WindowSize = SurfCap.currentExtent;
}

void UIRenderer::CreatePipeline()
{
  VkViewport Viewport{};
  VkRect2D Scissor{};
  VkPipelineViewportStateCreateInfo ViewportState{};

  {
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = WindowSize.width;
    Viewport.height = WindowSize.height;
    Viewport.minDepth = 0.f;
    Viewport.maxDepth = 1.f;

    Scissor.extent = WindowSize;
    Scissor.offset = {0, 0};

    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissor;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
  }

  VkPipelineLayoutCreateInfo LayoutCI{};
  LayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  LayoutCI.setLayoutCount = 1;
  LayoutCI.pSetLayouts;

  VkGraphicsPipelineCreateInfo PipelineCI{};
  PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
}

