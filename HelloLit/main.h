#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>

#include <glm/glm.hpp>

// functions
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
  void InitDescriptorPool();
  void InitDescriptors();
  void InitSamplers();
  void InitSync();

  void SubmitRender(uint32_t RenderIndex);
  void Present(uint32_t RenderIndex);

  void UpdateDescriptors();
  void RecordRender(uint32_t ImageIndex);

  void CreateImage(VkImage* Image, VkDeviceMemory* Memory, VkExtent2D Resolution, VkFormat Format, VkImageLayout ImageLayout, VkImageUsageFlags Usage);
  void CreateImageView(VkImage* Image, VkImageView* View, VkFormat Format, VkImageAspectFlags Aspects);

  void CopyBuffer(VkBuffer* Src, VkBuffer* Dst, VkBufferCopy* CopyInfo);
  void CopyBufferImage(VkBuffer* Src, VkImage* Dst, VkImageLayout Layout, VkBufferImageCopy CopyInfo);

  void Cleanup();

  std::vector<VkVertexInputBindingDescription> GetBindingDescription();
  std::vector<VkVertexInputAttributeDescription> GetAttributeDescription();

// Definitions
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

// Packed Struct containing our MVP matrices
struct WVP
{
  glm::mat4 World;
  glm::mat4 View;
  glm::mat4 Proj;
};

// Heads up, classes aren't packed like structs, so things might get messy if you try to define the Vertetx as a class instead of a struct.
struct Vertex
{
  public:
    glm::vec3 vPos;
    glm::vec3 vColor;
    glm::vec3 vNorm;
    glm::vec2 vCoord;
};

// Camera class
class Camera
{
  public:
    WVP Matrices;

    Camera();

    void Update();

    glm::vec3 GetPos() { return Position; }
    glm::vec3 GetRot() { return Rotation; }

    glm::mat4 CameraMatrix;

  private:
    glm::vec3 Position;
    glm::vec3 Rotation;

    glm::vec2 LastMouse;

    bool WindowFocused = true;

    float MoveSpeed = 0.005f;
    float RotateSpeed = 0.01f;
};

struct Context
{
  // Vulkan stuff
  VkInstance Instance;

  VkPhysicalDevice PhysicalDevice;

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

  std::vector<VkDeviceMemory> DepthMemory;
  std::vector<VkImage> DepthImages;
  std::vector<VkImageView> DepthImageViews;

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

  VkDeviceMemory MeshVertexMemory;
  VkBuffer MeshVertexBuffer;

  VkDeviceMemory MeshIndexMemory;
  VkBuffer MeshIndexBuffer;

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

  // Descriptor Pool for our WVP Buffer and Sampler image
  VkDescriptorPool DescPool;

  // WVP Layout
  VkDescriptorSetLayout DescLayout;
  // Sampler layout
  VkDescriptorSetLayout SamplerLayout;

  // WVP Set
  VkDescriptorSet DescSet;

  // WVP Buffer
  VkBuffer mvpBuffer;
  VkDeviceMemory mvpMemory;

  // our stuff
  QueueFamily Families;
  DeviceMemory Memory;

  // Mesh Importer
  Assimp::Importer Importer;
};
// contains all the vulkan structures that will be used to render.

struct Constants
{
  glm::vec3 CameraPosition;
};

struct Mesh
{
  public:
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;

    VkImage Texture_Albedo;
    VkImageView View_Albedo;
    VkDeviceMemory Memory_Albedo;

    VkSampler Sampler_Albedo;
    VkDescriptorSet TextureDescriptor;
};

// Variables
extern VkResult Error;
extern Context VkContext;
extern Camera Cam;
extern Mesh PrimaryMesh;
extern Constants PushConstants;

