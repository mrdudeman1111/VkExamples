#pragma once

#include "UI/Widgets.hpp"
#include "UI/Primitives.hpp"

#include <map>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <GLFW/glfw3.h>

// EventClick-> Check location against panes
// if in pane: Pane will move by MousePos Delta for everytime that left mouse evaluates to GLFW_PRESS
// else ignore the event

// Widget::Position is the position of the top left corner of the widget

#define EK_FONT_SIZE_LARGE 1
#define EK_FONT_SIZE_MEDIUM 2
#define EK_FONT_SIZE_SMALL 3
#define EK_FONT_SIZE_VERY_SMALL 4

namespace UI
{
  namespace vulkan
  {
    class Backend
    {
      public:
        VkInstance* pInstance;
        VkDevice* pDevice;
        VkPhysicalDevice* pPhysicalDevice;
        VkQueue* pGraphicsQueue;

        VkRenderPass* pRenderpass;
        uint32_t Subpass;

        VkSwapchainKHR* pSwapchain;
    };

    class Texture
    {
      public:
        VkImageMemoryBarrier CreateBarrier(VkImageLayout Layout);

        VkImage Img;
        VkImageLayout CurrentLayout;
        VkImageView View;
        VkExtent2D Extent;
        uint32_t memOffset;
        uint32_t memSize;
    };
  }

  class Character
  {
    public:
      vulkan::Texture Texture;
      glm::ivec2 pixSize;
      glm::ivec2 pixPadding;
      uint32_t pixOffset;

      // Index in texture array of fragment shader
      uint32_t DescriptorIndex;
  };

  class CharacterSet
  {
    public:
      std::map<char, Character> Characters;

      VkDeviceMemory CharMemory;
      VkBuffer CharacterBuffer;
      VkSampler CharacterSampler;
  };

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

  class RenderInterface
  {
    public:
      GLFWwindow* Window;
      VkExtent2D WindowSize;
  };

  class Renderer : RenderInterface
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

    private:
      void CreatePrimitives();
      void InitStructures();

      /* vulkan stuff */
        vulkan::Backend Base;

        uint32_t GraphicsFamily;
        uint32_t ComputeFamily;
        uint32_t LocalMemory;
        uint32_t HostMemory;

        VkBuffer TransferBuffer;
        VkDeviceMemory TransferMemory;
        VkBuffer primBuffer;
        VkDeviceMemory primMemory;

        VkCommandPool GraphicsPool;

        VkFence TransferFence;
        VkCommandBuffer cmdTransfer;

    private:
        VkDescriptorPool ShaderPool;
        VkDescriptorSetLayout FontLayout;
        VkDescriptorSet FontDescriptor;

        // Pipeline stuff

          VkViewport Viewport;
          VkRect2D RenderScissor;
          VkPipelineViewportStateCreateInfo ViewStateInfo;
          VkPipelineDepthStencilStateCreateInfo DepthStateInfo;
          VkPipelineMultisampleStateCreateInfo MultiSampleStateInfo;
          VkPipelineRasterizationStateCreateInfo RasterStateInfo;
          VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateInfo;
          VkPipelineColorBlendAttachmentState ColorAttachment;
          VkPipelineColorBlendStateCreateInfo BlendStateInfo;

          VkShaderModule curveVertexShader;
          VkShaderModule curveFragmentShader;
          VkPipelineLayout curvePipeLayout;
          VkPipeline curvePipe;

          VkShaderModule charVertexShader;
          VkShaderModule charFragmentShader;
          VkPipelineLayout charPipeLayout;
          VkPipeline charPipe;

          VkShaderModule rectVertexShader;
          VkShaderModule rectFragmentShader;
          VkPipelineLayout rectPipeLayout;
          VkPipeline rectPipe;

        /* UI Stuff */
          VkDescriptorSetLayout CharLayout;
          VkDescriptorPool CharPool;
          CharacterSet CharSet;
          std::vector<Widgets::Text> TextWidgets;

          VkDescriptorSetLayout BoxLayout;
          VkDescriptorPool BoxPool;
          std::vector<Widgets::Box> BoxWidgets;

        // GPU
          Primitives::PrimitiveBuffer PrimitiveData;
  };
}

