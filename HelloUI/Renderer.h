#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Rendering
{
  struct RendererCreateInfo
  {
    VkInstance Instance;
    VkPhysicalDevice PhysicalDevice;
    uint32_t LocalMemory;
    uint32_t HostMemory;
    VkDevice Device;
    VkQueue GraphicsQueue;

    VkRenderPass RenderPass;
    uint32_t SubpassIndex;
  };

  struct Vertex
  {
    public:
      glm::vec3 Position;
      glm::vec2 UV;
  };

  class Mesh
  {
    public:
      std::vector<Vertex> Vertices;
      std::vector<uint32_t> Indices;
      glm::mat4 Transform;

      VkDeviceMemory Memory;
      VkBuffer Buffer;

      VkDescriptorSetLayout MeshLayout;
      VkDescriptorSet MeshDescriptor;
  };

  class UnlitRenderer
  {
    public:
      UnlitRenderer(RendererCreateInfo& CI);

      void LoadMesh(const char* MeshPath);
      void Draw();
    private:
      VkInstance Instance;
      VkPhysicalDevice PhysicalDevice;
      VkDevice Device;
      VkQueue GraphicsQueue;
      VkRenderPass RenderPass;

      uint32_t SubpassIndex;
      uint32_t HostMemory;
      uint32_t LocalMemory;

      std::vector<Mesh> Meshes;
  };
}

