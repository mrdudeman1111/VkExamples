#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vulkan/vulkan.h>

#include <sail/sail/sail.h>
#include <sail/sail-manip/convert.h>

#include "main.h"

int main()
{
  // this function initiates our Window Library, so we can create a window on the screen with one function later.
  glfwInit();

  // Initate Instance.
  // The Instance is a handle to this application's vulkan state.
  // It also verifies that all Requested Instance Layers and extensions are available.
  // if available it assembles an instance call chain. for more info Look into
  // VkInstance
  // VkInstanceCreateInfo
  // vkCreateInstance
  // vulkan Loader
  // Vulkan Trampoline
  // Vulkan Instance call chain
  InitInstance();

  // Find Dedicated GPU, also query Queue Families.
  // If we need to get Hardware Info (Like VRAM Size, Bus width, etc)
  // we can do that at any point in the program, after we find the GPU we want to use.
  InitPhysicalDevice();

  // Initiate a logical Device.
  // The Device is a handle to the applications device state and connection to the physical device.
  // Similar to the Instance Initiation, but instad validates Device Instances and Extensions.
  // if available, the call Proceeds to assemble the device call chain.
  InitDevice();

  // these are the window dimensions we will use for the program.
  VkExtent2D WindowSize = {1280, 720};

  // Create Surface 
  // Create Swapchain
  InitSwapchain(WindowSize);

  // Create primary renderpass
  // with one subpass for mesh rendering
  InitRenderpass();

  // Init frambuffer with color attachment (screen)
  // and depth attachment (depth stencil for facial culling)
  InitFrameBuffer();

  // Init Command pool for GPU processes like copies and rendering
  InitCommandPool();

  // Init Command Buffers for memory operations and rendering operations
  InitCommandBuffer();

  // Read and create shaders for pipeline
  InitShaders();

  // create main pipeline with shaders from before
  InitPipeline();

  // Create structures needed for execution synchronization
  // so that we have memory safety
  InitSync();

  // Initiate mesh vertices, indices, colors, coordinates, and textures.
  InitMesh();

  // Initiate descriptor pool for passing buffers and textures to shaders.
  InitDescriptorPool();

  // initiate descriptors for buffers, like the camera buffer
  InitDescriptors();

  // Initiate descriptors for Textures, like the Mesh Albedo texture
  InitSamplers();

  // Index of render target (Framebuffer)
  uint32_t ImageIndex;

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwSetInputMode(VkContext.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Render loop
  while(!glfwWindowShouldClose(VkContext.Window))
  {
    glfwPollEvents();
    Cam.Update();
    UpdateDescriptors();

    vkAcquireNextImageKHR(VkContext.Device, VkContext.Swapchain, UINT64_MAX, VK_NULL_HANDLE, VkContext.ImageFence, &ImageIndex);

    vkWaitForFences(VkContext.Device, 1, &VkContext.ImageFence, VK_TRUE, UINT64_MAX);
    vkResetFences(VkContext.Device, 1, &VkContext.ImageFence);

    vkWaitForFences(VkContext.Device, 1, &VkContext.CmdFence[ImageIndex], VK_TRUE, UINT64_MAX);
    vkResetFences(VkContext.Device, 1, &VkContext.CmdFence[ImageIndex]);

    RecordRender(ImageIndex);

    SubmitRender(ImageIndex);

    printf("%f, %f, %f\n", PushConstants.CameraPosition.x, PushConstants.CameraPosition.y, PushConstants.CameraPosition.z);

    Present(ImageIndex);


    //vkWaitForFences(VkContext.Device, 1, &VkContext.CmdFence[ImageIndex], VK_TRUE, UINT64_MAX);
  }

  vkDeviceWaitIdle(VkContext.Device);

  Cleanup();

  std::cout << "Succesfull run\n";
  return 0;
}

void InitPipeline()
{
  // setup descriptor binding for the MVP and albedo sampler
  {
    VkDescriptorSetLayoutBinding UniformBinding{};
    UniformBinding.descriptorCount = 1;
    UniformBinding.binding = 0;
    UniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    UniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = 1;
    LayoutCI.pBindings = &UniformBinding;

    if((Error = vkCreateDescriptorSetLayout(VkContext.Device, &LayoutCI, nullptr, &VkContext.DescLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Error));
    }
  }

  // Setup descriptor binding for the Sampler
  {
    VkDescriptorSetLayoutBinding ImageBinding{};
    ImageBinding.descriptorCount = 1;
    ImageBinding.binding = 0;
    ImageBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    ImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = 1;
    LayoutCI.pBindings = &ImageBinding;

    if((Error = vkCreateDescriptorSetLayout(VkContext.Device, &LayoutCI, nullptr, &VkContext.SamplerLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Error));
    }
  }

  VkDescriptorSetLayout Layouts[] = {VkContext.DescLayout, VkContext.SamplerLayout};

  VkPushConstantRange ConstantsRange{};
  ConstantsRange.size = sizeof(PushConstants);
  ConstantsRange.offset = 0;
  ConstantsRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  // Create the pipeline layout
  {
    VkPipelineLayoutCreateInfo PipeLayoutInfo{};
    PipeLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipeLayoutInfo.setLayoutCount = 2;
    PipeLayoutInfo.pSetLayouts = Layouts;
    PipeLayoutInfo.pushConstantRangeCount = 1;
    PipeLayoutInfo.pPushConstantRanges = &ConstantsRange;

    if((Error = vkCreatePipelineLayout(VkContext.Device, &PipeLayoutInfo, nullptr, &VkContext.PipelineLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create pipeline layout with error: " + std::to_string(Error));
    }
  }

  VkRect2D Scissor{};
  Scissor.extent.width = 1280;
  Scissor.extent.height = 720;
  Scissor.offset.x = 0;
  Scissor.offset.y = 0;

  VkViewport Viewport{};
  Viewport.y = 0;
  Viewport.x = 0;
  Viewport.width = 1280;
  Viewport.height = 720;
  Viewport.minDepth = 0.f;
  Viewport.maxDepth = 1.f;

  VkPipelineViewportStateCreateInfo PipelineViewport{};

  // Viewport
  {
    PipelineViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    PipelineViewport.scissorCount = 1;
    PipelineViewport.pScissors = &Scissor;
    PipelineViewport.viewportCount = 1;
    PipelineViewport.pViewports = &Viewport;
  }

  VkPipelineInputAssemblyStateCreateInfo InputAssembly{};

  // InputAssembly (topology type, Index type, etc)
  {
    InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssembly.primitiveRestartEnable = VK_FALSE;
  }

  VkPipelineShaderStageCreateInfo PipelineShaderStages[2]{};

  // Shader stages (Vertex, Fragment)
  {
    PipelineShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    PipelineShaderStages[0].pName = "main";
    PipelineShaderStages[0].module = VkContext.VertexShader;

    PipelineShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    PipelineShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    PipelineShaderStages[1].pName = "main";
    PipelineShaderStages[1].module = VkContext.FragmentShader;
  }

  VkPipelineColorBlendAttachmentState ColorAttachment{};
  VkPipelineColorBlendStateCreateInfo ColorBlendState{};

  // Color blending
  {
    ColorAttachment.blendEnable = VK_FALSE;
    ColorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    ColorBlendState.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendState.logicOpEnable = VK_FALSE;

    ColorBlendState.blendConstants[0] = 0.f;
    ColorBlendState.blendConstants[1] = 0.f;
    ColorBlendState.blendConstants[2] = 0.f;
    ColorBlendState.blendConstants[3] = 0.f;

    ColorBlendState.attachmentCount = 1;
    ColorBlendState.pAttachments = &ColorAttachment;
  }

  VkPipelineVertexInputStateCreateInfo VertexInput{};
    std::vector<VkVertexInputAttributeDescription> VertexAttribute;
    std::vector<VkVertexInputBindingDescription> VertexBinding;

  // Setup Vertex Input
  {
    VertexAttribute = GetAttributeDescription();
    VertexBinding = GetBindingDescription();

    VertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInput.vertexBindingDescriptionCount = VertexBinding.size();
    VertexInput.vertexAttributeDescriptionCount = VertexAttribute.size();
    VertexInput.pVertexBindingDescriptions = VertexBinding.data();
    VertexInput.pVertexAttributeDescriptions = VertexAttribute.data();
  }

  VkPipelineDepthStencilStateCreateInfo DepthState{};

  // Depth stencil testing
  {
    DepthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthState.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthState.minDepthBounds = 0.f;
    DepthState.maxDepthBounds = 1.f;
    DepthState.depthTestEnable = VK_TRUE;
    DepthState.depthWriteEnable = VK_TRUE;
    DepthState.stencilTestEnable = VK_FALSE;
    DepthState.depthBoundsTestEnable = VK_FALSE;
  }

  VkPipelineRasterizationStateCreateInfo RasterState{};

  // Rasterization Info
  {
    RasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterState.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterState.lineWidth = 1.f;
    RasterState.polygonMode = VK_POLYGON_MODE_FILL;

    RasterState.depthBiasEnable = VK_FALSE;
    RasterState.depthClampEnable = VK_FALSE;
  }

  VkPipelineMultisampleStateCreateInfo SampleState{};

  // MultiSample settings for pipelines
  {
    SampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    SampleState.minSampleShading = 1.f;
    SampleState.alphaToOneEnable = false;
    SampleState.alphaToCoverageEnable = VK_FALSE;
    SampleState.sampleShadingEnable = VK_FALSE;
    SampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  }

  VkGraphicsPipelineCreateInfo PipelineCI{};

  // Pipeline Creation
  {
    PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCI.layout = VkContext.PipelineLayout;
    PipelineCI.renderPass = VkContext.Renderpass;
    PipelineCI.subpass = 0;
    PipelineCI.stageCount = 2;
    PipelineCI.pStages = PipelineShaderStages;
    PipelineCI.pInputAssemblyState = &InputAssembly;
    PipelineCI.pViewportState = &PipelineViewport;
    PipelineCI.pColorBlendState = &ColorBlendState;
    PipelineCI.pVertexInputState = &VertexInput;
    PipelineCI.pDepthStencilState = &DepthState;
    PipelineCI.pRasterizationState = &RasterState;
    PipelineCI.pMultisampleState = &SampleState;
  }

  if((Error = vkCreateGraphicsPipelines(VkContext.Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &VkContext.Pipeline)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create Graphics pipeline with error: " + std::to_string(Error));
  }
}

void InitMesh()
{
  // Initiate Transfer Buffer
  {
    VkBufferCreateInfo tBufferInfo{};
    tBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    tBufferInfo.size = 512000000; // 512 MB
    tBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    tBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Error = vkCreateBuffer(VkContext.Device, &tBufferInfo, nullptr, &VkContext.TransitBuffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create transit buffer with error: " + std::to_string(Error));
    }

    VkMemoryRequirements tMemReq{};
    vkGetBufferMemoryRequirements(VkContext.Device, VkContext.TransitBuffer, &tMemReq);

    VkMemoryAllocateInfo tMemAlloc{};
    tMemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    tMemAlloc.allocationSize = tMemReq.size;
    tMemAlloc.memoryTypeIndex = VkContext.Memory.Visible;

    if((Error = vkAllocateMemory(VkContext.Device, &tMemAlloc, nullptr, &VkContext.TransitMemory)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate memory for transit buffer with error: " + std::to_string(Error));
    }

    if((Error = vkBindBufferMemory(VkContext.Device, VkContext.TransitBuffer, VkContext.TransitMemory, 0)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind transit buffer to transit memory with error: " + std::to_string(Error));
    }
  }

  // Load Mesh file
  const aiScene* pScene = VkContext.Importer.ReadFile("Mesh.dae", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

  // check for meshes, if none, then throw error
  if(pScene->HasMeshes())
  {
    // Prepare vertex array
    PrimaryMesh.Vertices.resize(pScene->mMeshes[0]->mNumVertices);

    // Fill Vertex array and vertex colors if there are any
    for(uint32_t i = 0; i < pScene->mMeshes[0]->mNumVertices; i++)
    {
      PrimaryMesh.Vertices[i].vPos.x = pScene->mMeshes[0]->mVertices[i].y;
      PrimaryMesh.Vertices[i].vPos.y = pScene->mMeshes[0]->mVertices[i].z;
      PrimaryMesh.Vertices[i].vPos.z = -1.f*pScene->mMeshes[0]->mVertices[i].x;

      if(pScene->mMeshes[0]->mTextureCoords[0])
      {
        // account for opengl vs vulkan texture coordinates by inverting the y coordinate
        PrimaryMesh.Vertices[i].vCoord.x = pScene->mMeshes[0]->mTextureCoords[0][i].x;
        PrimaryMesh.Vertices[i].vCoord.y = -1*pScene->mMeshes[0]->mTextureCoords[0][i].y;
      }

      PrimaryMesh.Vertices[i].vNorm.x = pScene->mMeshes[0]->mNormals[i].y;
      PrimaryMesh.Vertices[i].vNorm.y = pScene->mMeshes[0]->mNormals[i].z;
      PrimaryMesh.Vertices[i].vNorm.z = -1.f*pScene->mMeshes[0]->mNormals[i].x;

      if(pScene->mMeshes[0]->HasVertexColors(0))
      {
        PrimaryMesh.Vertices[i].vColor.r = pScene->mMeshes[0]->mColors[0][i].r;
        PrimaryMesh.Vertices[i].vColor.g = pScene->mMeshes[0]->mColors[0][i].g;
        PrimaryMesh.Vertices[i].vColor.b = pScene->mMeshes[0]->mColors[0][i].b;
      }
    }

    // Fill Index Array
    for(uint32_t i = 0; i < pScene->mMeshes[0]->mNumFaces; i++)
    {
      // if the number of Indices isn't 3. Then it's not a triangle, we don't want that.
      // the pipeline topology type is VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST meaning we can only assemble triangles
      assert(pScene->mMeshes[0]->mFaces[i].mNumIndices == 3);

      PrimaryMesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[0]);
      PrimaryMesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[1]);
      PrimaryMesh.Indices.push_back(pScene->mMeshes[0]->mFaces[i].mIndices[2]);
    }

    // Load textures
    {
      aiString AlbedoPath;
      pScene->mMaterials[pScene->mMeshes[0]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &AlbedoPath);

      sail_image *RawImage, *Image;
      sail_load_from_file(AlbedoPath.C_Str(), &RawImage);

      sail_convert_image_with_options(RawImage, SAIL_PIXEL_FORMAT_BPP32_RGBA, NULL, &Image);

      CreateImage(&PrimaryMesh.Texture_Albedo, &PrimaryMesh.Memory_Albedo, VkExtent2D{Image->width, Image->height}, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      CreateImageView(&PrimaryMesh.Texture_Albedo, &PrimaryMesh.View_Albedo, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

      // Layout Transition
      {
        VkImageMemoryBarrier ImgBarrier{};
        ImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        ImgBarrier.image = PrimaryMesh.Texture_Albedo;
        ImgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ImgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
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

      // Copy buffer to image
      {
        void* pTransit;
        vkMapMemory(VkContext.Device, VkContext.TransitMemory, 0, 512000000, 0, &pTransit);
        memcpy((uint32_t*)pTransit, (uint32_t*)Image->pixels, (Image->width*Image->height*sizeof(uint32_t)));

        VkBufferImageCopy CopyInfo{};

        // fill copy info
        {
          CopyInfo.imageExtent = {(uint32_t)Image->width, (uint32_t)Image->height, 1};
          CopyInfo.imageOffset = VkOffset3D{0, 0, 0};
          CopyInfo.bufferOffset = 0;

          CopyInfo.imageSubresource.layerCount = 1;
          CopyInfo.imageSubresource.mipLevel = 0;
          CopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
          CopyInfo.imageSubresource.baseArrayLayer = 0;
        }

        CopyBufferImage(&VkContext.TransitBuffer, &PrimaryMesh.Texture_Albedo, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, CopyInfo);

        vkWaitForFences(VkContext.Device, 1, &VkContext.MemoryFence, VK_TRUE, UINT64_MAX);
        vkResetFences(VkContext.Device, 1, &VkContext.MemoryFence);
      }

      vkUnmapMemory(VkContext.Device, VkContext.TransitMemory);
    }

    // Create vertex buffer
    {
      // Define Buffer
      VkBufferCreateInfo vBufferInfo{};
      vBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      vBufferInfo.size = sizeof(Vertex)*PrimaryMesh.Vertices.size();
      vBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      vBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      // Create buffer
      if((Error = vkCreateBuffer(VkContext.Device, &vBufferInfo, nullptr, &VkContext.MeshVertexBuffer)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create triangle vertex buffer with error: " + std::to_string(Error));
      }

      // Get alignment, size, and memory type requirements
      VkMemoryRequirements vMemReq;
      vkGetBufferMemoryRequirements(VkContext.Device, VkContext.MeshVertexBuffer, &vMemReq);

      // Define memory allocation
      VkMemoryAllocateInfo vAllocInfo{};
      vAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      vAllocInfo.pNext = nullptr;
      vAllocInfo.allocationSize = vMemReq.size;
      vAllocInfo.memoryTypeIndex = VkContext.Memory.Local;

      // Allocate memory
      if((Error = vkAllocateMemory(VkContext.Device, &vAllocInfo, nullptr, &VkContext.MeshVertexMemory)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate Triangle Vertex memory with error: " + std::to_string(Error));
      }

      // Bind memory
      if((Error = vkBindBufferMemory(VkContext.Device, VkContext.MeshVertexBuffer, VkContext.MeshVertexMemory, 0)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to bind Triangle vertex buffer to memory with error: " + std::to_string(Error));
      }
    }

    // Repeat with Index buffer
    {
      VkBufferCreateInfo iBufferInfo{};
      iBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      iBufferInfo.size = sizeof(uint32_t) * PrimaryMesh.Indices.size();
      iBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      iBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if((Error = vkCreateBuffer(VkContext.Device, &iBufferInfo, nullptr, &VkContext.MeshIndexBuffer)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create Triangle Index buffer with error: " + std::to_string(Error));
      }

      VkMemoryRequirements iMemReq;
      vkGetBufferMemoryRequirements(VkContext.Device, VkContext.MeshIndexBuffer, &iMemReq);

      VkMemoryAllocateInfo iAllocInfo{};
      iAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      iAllocInfo.pNext = nullptr;
      iAllocInfo.allocationSize = iMemReq.size;
      iAllocInfo.memoryTypeIndex = VkContext.Memory.Local;

      if(iMemReq.memoryTypeBits & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) std::cout << "Index buffer supports Local\n";

      if((Error = vkAllocateMemory(VkContext.Device, &iAllocInfo, nullptr, &VkContext.MeshIndexMemory)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate Triangle Index Memory with error: " + std::to_string(Error));
      }

      if((Error = vkBindBufferMemory(VkContext.Device, VkContext.MeshIndexBuffer, VkContext.MeshIndexMemory, 0)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to bind Triangle index memory with error: " + std::to_string(Error));
      }
    }

    std::cout << "Mesh loaded\n";
  }
  else
  {
    throw std::runtime_error("Tried to load mesh file with no meshes");
  }

  // Map Transit Memory
  void* TransitMemory;
  vkMapMemory(VkContext.Device, VkContext.TransitMemory, 0, 512000000, 0, &TransitMemory);

  // Fill Transit Buffer with Vertices
  {
    memcpy((Vertex*)TransitMemory, PrimaryMesh.Vertices.data(), sizeof(Vertex)*PrimaryMesh.Vertices.size());
  }

  // Transfer Vertices from Transit to Vertex Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(Vertex) * PrimaryMesh.Vertices.size();
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(VkContext.MemoryCmd, VkContext.TransitBuffer, VkContext.MeshVertexBuffer, 1, &CopyInfo);
    vkEndCommandBuffer(VkContext.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

    vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.MemoryFence);
  }

  // wait for transfer to finish
  {
    vkWaitForFences(VkContext.Device, 1, &VkContext.MemoryFence, VK_TRUE, UINT64_MAX);

    vkResetFences(VkContext.Device, 1, &VkContext.MemoryFence);

    vkResetCommandBuffer(VkContext.MemoryCmd, 0);
  }

  // Fill Transfer buffer with Indices
  {
    memcpy((uint32_t*)TransitMemory, PrimaryMesh.Indices.data(), sizeof(uint32_t)*PrimaryMesh.Indices.size());
  }

  // Copy Indices from Transit to Index Buffer
  {
    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy CopyInfo{};
    CopyInfo.size = sizeof(uint32_t) * PrimaryMesh.Indices.size();
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(VkContext.MemoryCmd, VkContext.TransitBuffer, VkContext.MeshIndexBuffer, 1, &CopyInfo);
    vkEndCommandBuffer(VkContext.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

    vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.MemoryFence);
  }

  // Wait for transfer to finish
  {
    vkWaitForFences(VkContext.Device, 1, &VkContext.MemoryFence, VK_TRUE, UINT64_MAX);

    vkResetFences(VkContext.Device, 1, &VkContext.MemoryFence);

    vkResetCommandBuffer(VkContext.MemoryCmd, 0);
  }

  vkUnmapMemory(VkContext.Device, VkContext.TransitMemory);
}

void RecordRender(uint32_t ImageIndex)
{
  // record the render command for each frame buffer
  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.pNext = nullptr;

  VkClearValue ColorClearValue{};

  // Color Values
  ColorClearValue.color.int32[0] = 0; ColorClearValue.color.int32[1] = 0; ColorClearValue.color.int32[2] = 0; ColorClearValue.color.int32[3] = 0;

  VkClearValue DepthClearValue{};

  // Depth Values
  DepthClearValue.depthStencil.depth = 1.f;

  // a clear value for each attachment
  VkClearValue ClearValues[] = {ColorClearValue, DepthClearValue};

  VkRenderPassBeginInfo RenderPassInfo{};
  RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  RenderPassInfo.pNext = nullptr;
  RenderPassInfo.renderPass = VkContext.Renderpass;

  RenderPassInfo.renderArea.extent = {1280, 720};
  RenderPassInfo.renderArea.offset = {0, 0};

  RenderPassInfo.clearValueCount = 2;
  RenderPassInfo.pClearValues = ClearValues;

  RenderPassInfo.framebuffer = VkContext.FrameBuffers[ImageIndex];

  vkBeginCommandBuffer(VkContext.RenderBuffers[ImageIndex], &BeginInfo);

    vkCmdBeginRenderPass(VkContext.RenderBuffers[ImageIndex], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      VkDeviceSize Offset = 0;

      // Bind the Mesh information and pipeline.
      vkCmdBindVertexBuffers(VkContext.RenderBuffers[ImageIndex], 0, 1, &VkContext.MeshVertexBuffer, &Offset);
      vkCmdBindIndexBuffer(VkContext.RenderBuffers[ImageIndex], VkContext.MeshIndexBuffer, Offset, VK_INDEX_TYPE_UINT32);

      vkCmdBindDescriptorSets(VkContext.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VkContext.PipelineLayout, 0, 1, &VkContext.DescSet, 0, nullptr);
      vkCmdBindDescriptorSets(VkContext.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VkContext.PipelineLayout, 1, 1, &PrimaryMesh.TextureDescriptor, 0, nullptr);

      vkCmdPushConstants(VkContext.RenderBuffers[ImageIndex], VkContext.PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &PushConstants);

      vkCmdBindPipeline(VkContext.RenderBuffers[ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, VkContext.Pipeline);

      // Draw the mesh
      vkCmdDrawIndexed(VkContext.RenderBuffers[ImageIndex], PrimaryMesh.Indices.size()/*as specified in Triangle's struct definition*/, 1, 0, 0, 0);

    vkCmdEndRenderPass(VkContext.RenderBuffers[ImageIndex]);

  vkEndCommandBuffer(VkContext.RenderBuffers[ImageIndex]);
}

void UpdateDescriptors()
{
  // Update Buffer
  {
    void* TransferMemory;
    vkMapMemory(VkContext.Device, VkContext.TransitMemory, 0, sizeof(WVP), 0, &TransferMemory);

    memcpy(TransferMemory, &Cam.Matrices, sizeof(Cam.Matrices));


    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkBufferCopy Copy{};
    Copy.size = sizeof(Cam.Matrices);
    Copy.srcOffset = 0;
    Copy.dstOffset = 0;

    vkBeginCommandBuffer(VkContext.MemoryCmd, &BeginInfo);
      vkCmdCopyBuffer(VkContext.MemoryCmd, VkContext.TransitBuffer, VkContext.mvpBuffer, 1, &Copy);
    vkEndCommandBuffer(VkContext.MemoryCmd);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &VkContext.MemoryCmd;

    vkQueueSubmit(VkContext.GraphicsQueue, 1, &SubmitInfo, VkContext.MemoryFence);
  }

  // Update Descriptor
  {
    VkDescriptorBufferInfo BuffInfo{};
    BuffInfo.range = sizeof(WVP);
    BuffInfo.offset = 0;
    BuffInfo.buffer = VkContext.mvpBuffer;

    VkWriteDescriptorSet WriteInfo{};
    WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    WriteInfo.descriptorCount = 1;

    WriteInfo.dstSet = VkContext.DescSet;
    WriteInfo.dstBinding = 0;

    WriteInfo.pBufferInfo = &BuffInfo;

    vkWaitForFences(VkContext.Device, 1, &VkContext.MemoryFence, VK_TRUE, UINT64_MAX);
    vkResetFences(VkContext.Device, 1, &VkContext.MemoryFence);

    vkUpdateDescriptorSets(VkContext.Device, 1, &WriteInfo, 0, nullptr);
  }

  vkUnmapMemory(VkContext.Device, VkContext.TransitMemory);

  // Update push constants
  {
    PushConstants.CameraPosition = Cam.GetPos();
  }
}

void Cleanup()
{
  vkDestroySampler(VkContext.Device, PrimaryMesh.Sampler_Albedo, nullptr);

  vkDestroyImageView(VkContext.Device, PrimaryMesh.View_Albedo, nullptr);
  vkDestroyImage(VkContext.Device, PrimaryMesh.Texture_Albedo, nullptr);
  vkFreeMemory(VkContext.Device, PrimaryMesh.Memory_Albedo, nullptr);

  vkFreeCommandBuffers(VkContext.Device, VkContext.GraphicsPool, VkContext.RenderBuffers.size(), VkContext.RenderBuffers.data());
  vkFreeCommandBuffers(VkContext.Device, VkContext.GraphicsPool, 1, &VkContext.MemoryCmd);
  vkDestroyCommandPool(VkContext.Device, VkContext.GraphicsPool, nullptr);

  vkDestroySemaphore(VkContext.Device, VkContext.RenderSemaphore, nullptr);
  vkDestroyFence(VkContext.Device, VkContext.ImageFence, nullptr);
  vkDestroyFence(VkContext.Device, VkContext.MemoryFence, nullptr);

  for(uint32_t i = 0; i < VkContext.CmdFence.size(); i++)
  {
    vkDestroyFence(VkContext.Device, VkContext.CmdFence[i], nullptr);
  }

  vkDestroyBuffer(VkContext.Device, VkContext.TransitBuffer, nullptr);
  vkFreeMemory(VkContext.Device, VkContext.TransitMemory, nullptr);

  vkDestroyBuffer(VkContext.Device, VkContext.MeshVertexBuffer, nullptr);
  vkFreeMemory(VkContext.Device, VkContext.MeshVertexMemory, nullptr);

  vkDestroyBuffer(VkContext.Device, VkContext.MeshIndexBuffer, nullptr);
  vkFreeMemory(VkContext.Device, VkContext.MeshIndexMemory, nullptr);

  vkDestroyBuffer(VkContext.Device, VkContext.mvpBuffer, nullptr);
  vkFreeMemory(VkContext.Device, VkContext.mvpMemory, nullptr);

  // This will destroy all descriptor sets allocated as well
  vkDestroyDescriptorPool(VkContext.Device, VkContext.DescPool, nullptr);
  vkDestroyDescriptorSetLayout(VkContext.Device, VkContext.DescLayout, nullptr);
  vkDestroyDescriptorSetLayout(VkContext.Device, VkContext.SamplerLayout, nullptr);

  vkDestroyPipeline(VkContext.Device, VkContext.Pipeline, nullptr);
  vkDestroyPipelineLayout(VkContext.Device, VkContext.PipelineLayout, nullptr);

  vkDestroyShaderModule(VkContext.Device, VkContext.VertexShader, nullptr);
  vkDestroyShaderModule(VkContext.Device, VkContext.FragmentShader, nullptr);

  for(uint32_t i = 0; i < VkContext.FrameBufferCount; i++)
  {
    vkDestroyFramebuffer(VkContext.Device, VkContext.FrameBuffers[i], nullptr);
  }

  for(uint32_t i = 0; i < VkContext.SwapchainImageViews.size(); i++)
  {
    vkDestroyImageView(VkContext.Device, VkContext.SwapchainImageViews[i], nullptr);
    vkDestroyImageView(VkContext.Device, VkContext.DepthImageViews[i], nullptr);
  }

  for(uint32_t i = 0; i < VkContext.SwapchainImageViews.size(); i++)
  {
    vkDestroyImage(VkContext.Device, VkContext.DepthImages[i], nullptr);
    vkFreeMemory(VkContext.Device, VkContext.DepthMemory[i], nullptr);
  }

  vkDestroyRenderPass(VkContext.Device, VkContext.Renderpass, nullptr);

  // this will also destroy our swapchain images.
  vkDestroySwapchainKHR(VkContext.Device, VkContext.Swapchain, nullptr);

  vkDestroySurfaceKHR(VkContext.Instance, VkContext.Surface, nullptr);

  glfwDestroyWindow(VkContext.Window);
  glfwTerminate();

  // this destroys our queues too
  vkDestroyDevice(VkContext.Device, nullptr);

  vkDestroyInstance(VkContext.Instance, nullptr);
}

