#pragma once

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Primitives
{
  struct Plane
  {
    public:
      Plane();

      struct
      {
        glm::vec2 Pos;
        glm::vec2 UV;
      } Vertices[4];

      uint32_t Indices[6];
  };

  struct Triangle
  {
    public:
      Triangle();

      struct
      {
        glm::vec2 Pos;
        glm::vec2 UV;
      } Vertices[3];
  };

  struct PrimitiveBuffer
  {
    PrimitiveBuffer();

    Plane primPlane;
    Triangle primTriangle;
  };
}

