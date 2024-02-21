#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "UI/Widgets.hpp"

namespace UI
{
  struct RendererCreateInfo
  {
    public:
      GLFWwindow* pWindow;

      VkInstance* pInstance;
      VkPhysicalDevice* pPhysicalDevice;
      VkDevice* pDevice;
      VkQueue* pGraphicsQueue;
      VkRenderPass* pRenderpass;

      uint32_t HostMemoryIndex;
      uint32_t LocalMemoryIndex;

      uint32_t Subpass;
  };

  class Renderer
  {
    public:
      Renderer();

      void Init(RendererCreateInfo CreateInfo);
      void LoadFonts();
      void SetOutput(GLFWwindow* pWindow);
      void CreateUIPipeline();
      void CreateTrianglePipeline();
      void CreateRectPipeline();

      Widgets::Text* CreateTextWidget(glm::vec2 Position, glm::vec2 Size, uint32_t EK_FONT_SIZE, const char* Value);
      Widgets::Box* CreateBoxWidget(glm::vec2 Position, glm::vec2 Size, glm::vec3 Color, float smoothing);

      void UpdateWidgets();

      void DrawUI(VkCommandBuffer& cmdBuffer);
      void DrawTriangle(VkCommandBuffer& cmdBuffer);
      void DrawRect(VkCommandBuffer& cmdBuffer);
  };

  extern Renderer vkUI;
}

