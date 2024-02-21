#pragma once

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

/* Widgets */
namespace Widgets
{
  class Text
  {
    public:
      // in Pixels
      glm::vec2 pixPosition;
      glm::vec2 pixSize;
      uint32_t charDen;

      std::string strText;

      uint32_t charDrawCount;

      VkDeviceMemory instMemory;
      VkBuffer instBuffer;
  };

  class Widget
  {
    public:
      uint32_t Width;
      uint32_t Height;

      glm::vec2 Position;

      bool isCollide(glm::vec2 Position);
  };

  class Box 
  {
    public:
      VkDescriptorSet Descriptor;
      VkDeviceMemory DescriptorMemory;
      VkBuffer DescriptorBuffer;

      VkDeviceMemory instMemory;
      VkBuffer instBuffer;

      glm::vec2 pixPosition;
      glm::vec2 pixSize;

      glm::vec3 Color;

      float Smoothing;
  };
}

