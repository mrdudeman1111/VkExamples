#include "main.h"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <system_error>
#include <vector>
#include <iostream>
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace UI
{
  Renderer vkUI;
}

namespace Tools
{
  VkResult CreateImage(VkDevice& Device, VkImage& Image, VkFormat Format, glm::vec2 Size)
  {
    VkResult Err;

    VkImageCreateInfo ImageCI{}; ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCI.imageType = VK_IMAGE_TYPE_2D;
    ImageCI.format = Format;
    ImageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ImageCI.extent = {(uint32_t)Size.x, (uint32_t)Size.y, 1};
    ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCI.mipLevels = 1;
    ImageCI.arrayLayers = 1;

    if((Err = vkCreateImage(Device, &ImageCI, nullptr, &Image)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult CreateView(VkDevice& Device, VkImage& Image, VkImageView& View, VkFormat Format)
  {
    VkResult Err;

    VkImageViewCreateInfo ViewCI{};
    ViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewCI.image = Image;
    ViewCI.format = Format;
    ViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

    ViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
    ViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
    ViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
    ViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

    ViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ViewCI.subresourceRange.layerCount = 1;
    ViewCI.subresourceRange.levelCount = 1;
    ViewCI.subresourceRange.baseMipLevel = 0;
    ViewCI.subresourceRange.baseArrayLayer = 0;

    if((Err = vkCreateImageView(Device, &ViewCI, nullptr, &View)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  void CreateTexture(VkDevice& Device, UI::vulkan::Texture& Texture, VkFormat Format, glm::vec2 Size)
  {
    VkResult Err;

    Texture.Extent = {(uint32_t)Size.x, (uint32_t)Size.y};
    Texture.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if((Err = CreateImage(Device, Texture.Img, Format, Size)) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create texture with error: " + std::to_string(Err));
    }
  }

  VkResult Allocate(VkDevice& Device, uint32_t MemoryType, VkImage& Image, VkDeviceMemory& Memory)
  {
    VkResult Err;

    VkMemoryRequirements MemReq;
    vkGetImageMemoryRequirements(Device, Image, &MemReq);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemReq.size;
    AllocInfo.memoryTypeIndex = MemoryType;

    if((Err = vkAllocateMemory(Device, &AllocInfo, nullptr, &Memory)) != VK_SUCCESS)
    {
      return Err;
    }

    if((Err = vkBindImageMemory(Device, Image, Memory, 0)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult Allocate(VkDevice& Device, uint32_t MemoryType, VkBuffer& Buffer, VkDeviceMemory& Memory)
  {
    VkResult Err;

    VkMemoryRequirements MemReq;
    vkGetBufferMemoryRequirements(Device, Buffer, &MemReq);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemReq.size;
    AllocInfo.memoryTypeIndex = MemoryType;

    if((Err = vkAllocateMemory(Device, &AllocInfo, nullptr, &Memory)) != VK_SUCCESS)
    {
      return Err;
    }

    if((Err = vkBindBufferMemory(Device, Buffer, Memory, 0)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }
}

namespace UI
{
  namespace vulkan
  {
    VkImageMemoryBarrier Texture::CreateBarrier(VkImageLayout Layout)
    {
      VkImageMemoryBarrier Ret{};
      Ret.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      Ret.image = Img;
      Ret.oldLayout = CurrentLayout;
      Ret.newLayout = Layout;
      Ret.srcAccessMask = VK_ACCESS_NONE;
      Ret.dstAccessMask = VK_ACCESS_NONE;

      Ret.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      Ret.subresourceRange.layerCount = 1;
      Ret.subresourceRange.levelCount = 1;
      Ret.subresourceRange.baseMipLevel = 0;
      Ret.subresourceRange.baseArrayLayer = 0;

      CurrentLayout = Layout;

      return Ret;
    }
  }

  Renderer::Renderer()
  {

  }

  void Renderer::Init(RendererCreateInfo CreateInfo)
  {
    Base.pInstance = CreateInfo.pInstance;
    Base.pPhysicalDevice = CreateInfo.pPhysicalDevice;
    Base.pDevice = CreateInfo.pDevice;
    Base.pGraphicsQueue = CreateInfo.pGraphicsQueue;

    Window = CreateInfo.pWindow;

    int Width, Height;
    glfwGetWindowSize(Window, &Width, &Height);
    WindowSize = { (uint32_t)Width, (uint32_t)Height };

    Base.pRenderpass = CreateInfo.pRenderpass;
    Base.Subpass = CreateInfo.Subpass;

    HostMemory = CreateInfo.HostMemoryIndex;
    LocalMemory = CreateInfo.LocalMemoryIndex;

    uint32_t FamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(*Base.pPhysicalDevice, &FamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> Families(FamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(*Base.pPhysicalDevice, &FamilyCount, Families.data());

    GraphicsFamily = -1;
    ComputeFamily = -1;

    for(uint32_t i = 0; i < FamilyCount; i++)
    {
      if(Families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && GraphicsFamily == -1)
      {
        GraphicsFamily = i;
      }
      else if(Families[i].queueFlags & VK_QUEUE_COMPUTE_BIT && ComputeFamily == -1)
      {
        ComputeFamily = i;
      }
    }

    VkCommandPoolCreateInfo PoolCI{};
    PoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolCI.queueFamilyIndex = GraphicsFamily;
    PoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if(vkCreateCommandPool(*Base.pDevice, &PoolCI, nullptr, &GraphicsPool) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to initiate renderer, can't create graphics command pool");
    }

    InitStructures();
  }

  void Renderer::CreatePrimitives()
  {
    VkBufferCreateInfo PrimCI{};
    PrimCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    PrimCI.size = sizeof(PrimitiveData);
    PrimCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    PrimCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    vkCreateBuffer(*Base.pDevice, &PrimCI, nullptr, &primBuffer);

    std::cout << "Prim buffer size " << sizeof(PrimitiveData) << '\n';

    Tools::Allocate(*Base.pDevice, LocalMemory, primBuffer, primMemory);

    Primitives::PrimitiveBuffer* pStaging;
    vkMapMemory(*Base.pDevice, TransferMemory, 0, sizeof(PrimitiveData), 0, (void**)&pStaging);
      *pStaging = PrimitiveData;
    vkUnmapMemory(*Base.pDevice, TransferMemory);

    VkBufferCopy CopyInfo;
    CopyInfo.size = sizeof(PrimitiveData);
    CopyInfo.srcOffset = 0;
    CopyInfo.dstOffset = 0;

    VkCommandBufferBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(cmdTransfer, &BeginInfo);
      vkCmdCopyBuffer(cmdTransfer, TransferBuffer, primBuffer, 1, &CopyInfo);
    vkEndCommandBuffer(cmdTransfer);

    VkSubmitInfo SubmitInfo{};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &cmdTransfer;

    vkQueueSubmit(*Base.pGraphicsQueue, 1, &SubmitInfo, TransferFence);

    vkWaitForFences(*Base.pDevice, 1, &TransferFence, VK_TRUE, UINT64_MAX);
    vkResetFences(*Base.pDevice, 1, &TransferFence);
  }

  void Renderer::InitStructures()
  {
    VkCommandBufferAllocateInfo cmdAlloc{};
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAlloc.commandPool = GraphicsPool;
    cmdAlloc.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(*Base.pDevice, &cmdAlloc, &cmdTransfer) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to allocate transfer command buffer");
    }

    VkMemoryAllocateInfo CharAlloc{};
    CharAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    CharAlloc.memoryTypeIndex = LocalMemory;
    // 50 MB
    CharAlloc.allocationSize = 50000000;

    if(vkAllocateMemory(*Base.pDevice, &CharAlloc, nullptr, &CharSet.CharMemory) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to allocate character memory");
    }

    VkMemoryAllocateInfo TransferAlloc{};
    TransferAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    TransferAlloc.memoryTypeIndex = HostMemory;
    TransferAlloc.allocationSize = 10000000;

    if(vkAllocateMemory(*Base.pDevice, &TransferAlloc, nullptr, &TransferMemory) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to make transfer buffer");
    }


    VkBufferCreateInfo TransferCI{};
    TransferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    TransferCI.size = 10000000;
    TransferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    TransferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(*Base.pDevice, &TransferCI, nullptr, &TransferBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create transfer buffer");
    }

    vkBindBufferMemory(*Base.pDevice, TransferBuffer, TransferMemory, 0);

    VkFenceCreateInfo TransferFenceCI{};
    TransferFenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if(vkCreateFence(*Base.pDevice, &TransferFenceCI, nullptr, &TransferFence) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create transfer fence");
    }

    CreatePrimitives();

    // Create Box Widget Descriptor pool
    {
      VkDescriptorPoolSize BoxSize{};
      BoxSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      BoxSize.descriptorCount = 128;

      VkDescriptorPoolCreateInfo BoxPoolCI{};
      BoxPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      BoxPoolCI.pPoolSizes = &BoxSize;
      BoxPoolCI.poolSizeCount = 1;
      BoxPoolCI.maxSets = 128;

      if(vkCreateDescriptorPool(*Base.pDevice, &BoxPoolCI, nullptr, &BoxPool) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create box descriptor pool");
      }

      VkDescriptorSetLayoutBinding BoxBinding{};

      BoxBinding.binding = 0;
      BoxBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      BoxBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      BoxBinding.descriptorCount = 1;

      VkDescriptorSetLayoutCreateInfo BoxLayoutCI{};
      BoxLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      BoxLayoutCI.bindingCount = 1;
      BoxLayoutCI.pBindings = &BoxBinding;

      if(vkCreateDescriptorSetLayout(*Base.pDevice, &BoxLayoutCI, nullptr, &BoxLayout) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create box descriptor layout");
      }
    }

    // Create Pipeline Objects
    {
      Viewport.x = 0;
      Viewport.y = 0;
      Viewport.width = WindowSize.width;
      Viewport.height = WindowSize.height;
      Viewport.minDepth = 0.f;
      Viewport.maxDepth = 1.f;

      RenderScissor.offset = { 0, 0 };
      RenderScissor.extent = { WindowSize.width, WindowSize.height };

      ViewStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      ViewStateInfo.scissorCount = 1;
      ViewStateInfo.pScissors = &RenderScissor;
      ViewStateInfo.viewportCount = 1;
      ViewStateInfo.pViewports = &Viewport;

      DepthStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

      MultiSampleStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      MultiSampleStateInfo.sampleShadingEnable = VK_FALSE;
      MultiSampleStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      MultiSampleStateInfo.alphaToOneEnable = VK_FALSE;

      RasterStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      RasterStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
      RasterStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
      RasterStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
      RasterStateInfo.lineWidth = 1.f;
      RasterStateInfo.depthBiasEnable = false;
      RasterStateInfo.depthClampEnable = VK_FALSE;
      RasterStateInfo.rasterizerDiscardEnable = VK_FALSE;

      ColorAttachment.blendEnable = VK_TRUE;
      ColorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
      ColorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      ColorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
      ColorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
      ColorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
      ColorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
      ColorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

      BlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      BlendStateInfo.attachmentCount = 1;
      BlendStateInfo.pAttachments = &ColorAttachment;
      BlendStateInfo.logicOpEnable = VK_FALSE;

      InputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      InputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
  }

  void Renderer::LoadFonts()
  {
    FT_Library FtLib;
    FT_Face FtFace;
    FT_Init_FreeType(&FtLib);
    FT_New_Face(FtLib, FONT, 0, &FtFace); FT_Set_Pixel_Sizes(FtFace, 0, 48);

    uint32_t TransferOffset = 0;
    unsigned char* pTransfer;
    vkMapMemory(*Base.pDevice, TransferMemory, 0, 10000000, 0, (void**)&pTransfer);

    std::vector<std::pair<char, Character>> tmpChars;
    std::vector<uint32_t> tmpCharLocations;

    // fill glyph info and put raw image buffer in Transfer memory
      for(unsigned char i = 33; i < 126; i++)
      {
        FT_Load_Char(FtFace, i, FT_LOAD_RENDER);

        std::pair<char, Character> newChar;

        newChar.first = i;

        newChar.second.DescriptorIndex = tmpChars.size();
        newChar.second.pixSize.x = FtFace->glyph->bitmap.width;
        newChar.second.pixSize.y = FtFace->glyph->bitmap.rows;
        newChar.second.pixPadding = { FtFace->glyph->bitmap_left, FtFace->glyph->bitmap_top };
        newChar.second.pixOffset = FtFace->glyph->advance.x/64;

        uint32_t ImageSize = newChar.second.pixSize.x * newChar.second.pixSize.y;
        memcpy(pTransfer+TransferOffset, FtFace->glyph->bitmap.buffer, ImageSize);

        tmpChars.push_back(newChar);
        tmpCharLocations.push_back(TransferOffset);

        TransferOffset += ImageSize;
      }

      vkUnmapMemory(*Base.pDevice, TransferMemory);

    // Create Images and Views
      std::vector<VkBufferImageCopy> CopyInfos = {};
      uint32_t AllocOffset = 0;

      for(uint32_t i = 0; i < tmpChars.size(); i++)
      {
        Tools::CreateTexture(*Base.pDevice, tmpChars[i].second.Texture, VK_FORMAT_R8_UINT, tmpChars[i].second.pixSize);

        VkMemoryRequirements txMemReq{};
        vkGetImageMemoryRequirements(*Base.pDevice, tmpChars[i].second.Texture.Img, &txMemReq);

        AllocOffset += txMemReq.size;
        AllocOffset += AllocOffset % txMemReq.alignment; // ensure memory alignment
        vkBindImageMemory(*Base.pDevice, tmpChars[i].second.Texture.Img, CharSet.CharMemory, AllocOffset);

        Tools::CreateView(*Base.pDevice, tmpChars[i].second.Texture.Img, tmpChars[i].second.Texture.View, VK_FORMAT_R8_UINT);

        VkBufferImageCopy tmpCopy{};

        tmpCopy.imageExtent.width = tmpChars[i].second.pixSize.x;
        tmpCopy.imageExtent.height = tmpChars[i].second.pixSize.y;
        tmpCopy.imageExtent.depth = 1;

        tmpCopy.bufferOffset = tmpCharLocations[i];

        tmpCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        tmpCopy.imageSubresource.mipLevel = 0;
        tmpCopy.imageSubresource.layerCount = 1;
        tmpCopy.imageSubresource.baseArrayLayer = 0;

        CopyInfos.push_back(tmpCopy);
      }

    // Transfer layouts
      std::vector<VkImageMemoryBarrier> TransferBarriers = {};
      std::vector<VkImageMemoryBarrier> ReadableBarriers = {};

      for(uint32_t i = 0; i < tmpChars.size(); i++)
      {
        TransferBarriers.push_back(tmpChars[i].second.Texture.CreateBarrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL));

        ReadableBarriers.push_back(tmpChars[i].second.Texture.CreateBarrier(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
      }

    // record and execute command buffers to transfer layouts and copy images
      VkCommandBufferBeginInfo BeginInfo{};
      BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      VkSubmitInfo SubmitInfo{};
      SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      SubmitInfo.commandBufferCount = 1;
      SubmitInfo.pCommandBuffers = &cmdTransfer;

      vkBeginCommandBuffer(cmdTransfer, &BeginInfo);
        vkCmdPipelineBarrier(cmdTransfer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, TransferBarriers.size(), TransferBarriers.data());

        for(uint32_t i = 0; i < tmpChars.size(); i++)
        {
          vkCmdCopyBufferToImage(cmdTransfer, TransferBuffer, tmpChars[i].second.Texture.Img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyInfos[i]);
        }

        vkCmdPipelineBarrier(cmdTransfer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, ReadableBarriers.size(), ReadableBarriers.data());
      vkEndCommandBuffer(cmdTransfer);

      vkQueueSubmit(*Base.pGraphicsQueue, 1, &SubmitInfo, TransferFence);
      vkWaitForFences(*Base.pDevice, 1, &TransferFence, VK_TRUE, UINT64_MAX);

    // Create Descriptors
      VkDescriptorSetLayoutBinding Bindings[2] = {};
      VkDescriptorPoolSize Sizes[2] = {};

      Bindings[0].binding = 0;
      Bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      Bindings[0].descriptorCount = tmpChars.size();
      Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

      Bindings[1].binding = 1;
      Bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      Bindings[1].descriptorCount = 1;
      Bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

      Sizes[0].descriptorCount = tmpChars.size();
      Sizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

      Sizes[1].descriptorCount = 1;
      Sizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;

      VkDescriptorSetLayoutCreateInfo LayoutCI{};
      LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      LayoutCI.bindingCount = 2;
      LayoutCI.pBindings = Bindings;

      if(vkCreateDescriptorSetLayout(*Base.pDevice, &LayoutCI, nullptr, &FontLayout) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create descriptor set layout for fonts");
      }

      VkDescriptorPoolCreateInfo PoolCI{};
      PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      PoolCI.maxSets = 1;
      PoolCI.poolSizeCount = 2;
      PoolCI.pPoolSizes = Sizes;

      if(vkCreateDescriptorPool(*Base.pDevice, &PoolCI, nullptr, &ShaderPool) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create descriptor pool for fonts (ShaderPool)");
      }

      VkDescriptorSetAllocateInfo DescAlloc{};
      DescAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      DescAlloc.descriptorPool = ShaderPool;
      DescAlloc.descriptorSetCount = 1;
      DescAlloc.pSetLayouts = &FontLayout;

      if(vkAllocateDescriptorSets(*Base.pDevice, &DescAlloc, &FontDescriptor) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate font descriptor set");
      }

    // Create sampler
      VkSamplerCreateInfo SamplerCI{};
      SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      SamplerCI.minLod = 1.f;
      SamplerCI.maxLod = 1.f;
      SamplerCI.minFilter = VK_FILTER_NEAREST;
      SamplerCI.magFilter = VK_FILTER_NEAREST;
      SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      SamplerCI.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
      SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      SamplerCI.anisotropyEnable = VK_FALSE;

      if(vkCreateSampler(*Base.pDevice, &SamplerCI, nullptr, &CharSet.CharacterSampler) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create font sampler");
      }

    // Update Descriptor Sets
      std::vector<VkDescriptorImageInfo> DescImageInfos;

      for(uint32_t i = 0; i < tmpChars.size(); i++)
      {
        VkDescriptorImageInfo tmpInfo{};
        tmpInfo.imageView = tmpChars[i].second.Texture.View;
        tmpInfo.imageLayout = tmpChars[i].second.Texture.CurrentLayout;

        DescImageInfos.push_back(tmpInfo);
      }

      VkDescriptorImageInfo DescSamplerInfo{};
      DescSamplerInfo.sampler = CharSet.CharacterSampler;

      VkWriteDescriptorSet Writes[2] = {};

      Writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      Writes[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      Writes[0].descriptorCount = tmpChars.size();
      Writes[0].dstSet = FontDescriptor;
      Writes[0].dstBinding = 0;
      Writes[0].pImageInfo = DescImageInfos.data();
      Writes[0].dstArrayElement = 0;

      Writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      Writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
      Writes[1].descriptorCount = 1;
      Writes[1].dstSet = FontDescriptor;
      Writes[1].dstBinding = 1;
      Writes[1].pImageInfo = &DescSamplerInfo;
      Writes[1].dstArrayElement = 0;

      vkUpdateDescriptorSets(*Base.pDevice, 2, Writes, 0, nullptr);

      for(uint32_t i = 0; i < tmpChars.size(); i++)
      {
        CharSet.Characters[tmpChars[i].first] = tmpChars[i].second;
      }

      FT_Done_Face(FtFace);
      FT_Done_FreeType(FtLib);
  }

  void Renderer::CreateUIPipeline()
  {
    VkResult Err;

    VkShaderModule VertexShader;
    VkShaderModule FragmentShader;

    // Load shaders
    {
      VkShaderModuleCreateInfo VertexCI{};
      VkShaderModuleCreateInfo FragmentCI{};

      std::ifstream VertexFile(SHADERPATH"CharVert.spv", std::ios::ate);

      assert(VertexFile.is_open());

      uint32_t VertexSize = VertexFile.tellg();
      VertexFile.seekg(VertexFile.beg);

      std::vector<char> VertexCode(VertexSize);
      VertexFile.read(VertexCode.data(), VertexCode.size());

      VertexCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      VertexCI.codeSize = VertexSize;
      VertexCI.pCode = reinterpret_cast<const uint32_t*>(VertexCode.data());

      if((Err = vkCreateShaderModule(*Base.pDevice, &VertexCI, nullptr, &charVertexShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create curve vertex shader with error: " + std::to_string(Err));
      }

      std::ifstream FragmentFile(SHADERPATH"CharFrag.spv", std::ios::ate);

      assert(FragmentFile.is_open());

      uint32_t FragmentSize = FragmentFile.tellg();
      FragmentFile.seekg(FragmentFile.beg);

      std::vector<char> FragmentCode(FragmentSize);
      FragmentFile.read(FragmentCode.data(), FragmentCode.size());

      FragmentCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      FragmentCI.codeSize = FragmentSize;
      FragmentCI.pCode = reinterpret_cast<const uint32_t*>(FragmentCode.data());

      if((Err = vkCreateShaderModule(*Base.pDevice, &FragmentCI, nullptr, &charFragmentShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create curve fragment shader with error: " + std::to_string(Err));
      }
    }

    VkPipelineShaderStageCreateInfo ShaderStages[2] = {};
    VkVertexInputBindingDescription vtxBinding[2] = {};
    VkVertexInputAttributeDescription vtxAttributes[5] = {};
    VkPipelineVertexInputStateCreateInfo InputState{};

    {
      ShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      ShaderStages[0].pName = "main";
      ShaderStages[0].module = charVertexShader;

      ShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      ShaderStages[1].pName = "main";
      ShaderStages[1].module = charFragmentShader;

      vtxBinding[0].stride = sizeof(glm::vec2)+sizeof(glm::vec2);
      vtxBinding[0].binding = 0;
      vtxBinding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      vtxBinding[1].stride = sizeof(glm::vec2)+sizeof(glm::vec2)+sizeof(uint32_t);
      vtxBinding[1].binding = 1;
      vtxBinding[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

      vtxAttributes[0].binding = 0;
      vtxAttributes[0].location = 0;
      vtxAttributes[0].offset = 0;
      vtxAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[1].binding = 0;
      vtxAttributes[1].location = 1;
      vtxAttributes[1].offset = sizeof(glm::vec2);
      vtxAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[2].binding = 1;
      vtxAttributes[2].location = 2;
      vtxAttributes[2].offset = 0;
      vtxAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[3].binding = 1;
      vtxAttributes[3].location = 3;
      vtxAttributes[3].offset = sizeof(glm::vec2);
      vtxAttributes[3].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[4].binding = 1;
      vtxAttributes[4].location = 4;
      vtxAttributes[4].offset = sizeof(glm::vec2)*2;
      vtxAttributes[4].format = VK_FORMAT_R32_SINT;

      InputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      InputState.vertexBindingDescriptionCount = 2;
      InputState.pVertexBindingDescriptions = vtxBinding;
      InputState.vertexAttributeDescriptionCount = 5;
      InputState.pVertexAttributeDescriptions = vtxAttributes;
    }

    VkPipelineLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    LayoutCI.setLayoutCount = 1;
    LayoutCI.pSetLayouts = &FontLayout;

    if((Err = vkCreatePipelineLayout(*Base.pDevice, &LayoutCI, nullptr, &charPipeLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create pipeline layout with error: " + std::to_string(Err));
    }

    VkGraphicsPipelineCreateInfo PipelineCI{};
    PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCI.layout = charPipeLayout;
    PipelineCI.renderPass = *Base.pRenderpass;
    PipelineCI.subpass = Base.Subpass;
    PipelineCI.stageCount = 2;
    PipelineCI.pStages = ShaderStages;
    PipelineCI.pViewportState = &ViewStateInfo;
    PipelineCI.pDepthStencilState = &DepthStateInfo;
    PipelineCI.pMultisampleState = &MultiSampleStateInfo;
    PipelineCI.pVertexInputState = &InputState;
    PipelineCI.pInputAssemblyState = &InputAssemblyStateInfo;
    PipelineCI.pRasterizationState = &RasterStateInfo;
    PipelineCI.pColorBlendState = &BlendStateInfo;

    if((Err = vkCreateGraphicsPipelines(*Base.pDevice, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &charPipe)) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create ui pipeline with error: " + std::to_string(Err));
    }
  }

  void Renderer::CreateTrianglePipeline()
  {
    VkResult Err;

    VkShaderModuleCreateInfo VertexCI{};
    VkShaderModuleCreateInfo FragmentCI{};

    // Create Shaders
    {
      std::ifstream Vertex(SHADERPATH"CurveVert.spv", std::ifstream::ate);

      uint32_t VertexSize = Vertex.tellg();
      Vertex.seekg(Vertex.beg);

      const uint32_t* VertexCode = new uint32_t[VertexSize/sizeof(uint32_t)];

      Vertex.read((char*)VertexCode, VertexSize);

      VertexCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      VertexCI.codeSize = VertexSize;
      VertexCI.pCode = VertexCode;

      if((Err = vkCreateShaderModule(*Base.pDevice, &VertexCI, nullptr, &curveVertexShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create character vertex shader with error: " + std::to_string(Err));
      }

      std::ifstream Fragment(SHADERPATH"CurveFrag.spv", std::ifstream::ate);

      uint32_t FragmentSize = Fragment.tellg();
      Fragment.seekg(Fragment.beg);

      const uint32_t* FragmentCode = new uint32_t[FragmentSize/sizeof(uint32_t)];

      Fragment.read((char*)FragmentCode, FragmentSize);

      FragmentCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      FragmentCI.codeSize = FragmentSize;
      FragmentCI.pCode = FragmentCode;

      if((Err = vkCreateShaderModule(*Base.pDevice, &FragmentCI, nullptr, &curveFragmentShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create character fragment shader with error: " + std::to_string(Err));
      }
    }

    VkPipelineShaderStageCreateInfo ShaderStages[2] = {};
    VkPipelineVertexInputStateCreateInfo InputState{};
    VkVertexInputBindingDescription vtxBindings[1] = {};
    VkVertexInputAttributeDescription vtxAttributes[2] = {};

    {
      ShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[0].pName = "main";
      ShaderStages[0].module = curveVertexShader;
      ShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

      ShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[1].pName = "main";
      ShaderStages[1].module = curveFragmentShader;
      ShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

      vtxBindings[0].stride = sizeof(glm::vec2)*2;
      vtxBindings[0].binding = 0;
      vtxBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      vtxAttributes[0].binding = 0;
      vtxAttributes[0].location = 0;
      vtxAttributes[0].offset = 0;
      vtxAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[1].binding = 0;
      vtxAttributes[1].location = 1;
      vtxAttributes[1].offset = sizeof(glm::vec2);
      vtxAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;

      InputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      InputState.vertexBindingDescriptionCount = 1;
      InputState.pVertexBindingDescriptions = vtxBindings;
      InputState.vertexAttributeDescriptionCount = 2;
      InputState.pVertexAttributeDescriptions = vtxAttributes;
    }

    VkPipelineLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    LayoutCI.setLayoutCount = 0;
    LayoutCI.pSetLayouts = nullptr;
    LayoutCI.pushConstantRangeCount = 0;

    if((Err = vkCreatePipelineLayout(*Base.pDevice, &LayoutCI, nullptr, &curvePipeLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo PipelineCI{};
    PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCI.layout = curvePipeLayout;
    PipelineCI.renderPass = *Base.pRenderpass;
    PipelineCI.subpass = Base.Subpass;
    PipelineCI.stageCount = 2;
    PipelineCI.pStages = ShaderStages;
    PipelineCI.pViewportState = &ViewStateInfo;
    PipelineCI.pColorBlendState = &BlendStateInfo;
    PipelineCI.pMultisampleState = &MultiSampleStateInfo;
    PipelineCI.pVertexInputState = &InputState;
    PipelineCI.pDepthStencilState = &DepthStateInfo;
    PipelineCI.pInputAssemblyState = &InputAssemblyStateInfo;
    PipelineCI.pRasterizationState = &RasterStateInfo;

    if((Err = vkCreateGraphicsPipelines(*Base.pDevice, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &curvePipe)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create curve pipeline with error " + std::to_string(Err));
    }
  }

  void Renderer::CreateRectPipeline()
  {
    VkResult Err;

    VkShaderModuleCreateInfo VertexCI{};
    VkShaderModuleCreateInfo FragmentCI{};

    // Create Shaders
    {
      std::ifstream Vertex(SHADERPATH"RectVert.spv", std::ifstream::ate);

      uint32_t VertexSize = Vertex.tellg();
      Vertex.seekg(Vertex.beg);

      const uint32_t* VertexCode = new uint32_t[VertexSize/sizeof(uint32_t)];

      Vertex.read((char*)VertexCode, VertexSize);

      VertexCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      VertexCI.codeSize = VertexSize;
      VertexCI.pCode = VertexCode;

      if((Err = vkCreateShaderModule(*Base.pDevice, &VertexCI, nullptr, &rectVertexShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create character vertex shader with error: " + std::to_string(Err));
      }

      std::ifstream Fragment(SHADERPATH"RectFrag.spv", std::ifstream::ate);

      uint32_t FragmentSize = Fragment.tellg();
      Fragment.seekg(Fragment.beg);

      const uint32_t* FragmentCode = new uint32_t[FragmentSize/sizeof(uint32_t)];

      Fragment.read((char*)FragmentCode, FragmentSize);

      FragmentCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      FragmentCI.codeSize = FragmentSize;
      FragmentCI.pCode = FragmentCode;

      if((Err = vkCreateShaderModule(*Base.pDevice, &FragmentCI, nullptr, &rectFragmentShader)) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create character fragment shader with error: " + std::to_string(Err));
      }
    }

    VkPipelineLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    LayoutCI.setLayoutCount = 1;
    LayoutCI.pSetLayouts = &BoxLayout;

    if((Err = vkCreatePipelineLayout(*Base.pDevice, &LayoutCI, nullptr, &rectPipeLayout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create rect pipeline layout with error: " + std::to_string(Err));
    }

    VkPipelineShaderStageCreateInfo ShaderStages[2] = {};
    VkVertexInputBindingDescription vtxBinding[2] = {};
    VkVertexInputAttributeDescription vtxAttributes[4] = {};
    VkPipelineVertexInputStateCreateInfo InputState{};

    {
      ShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      ShaderStages[0].module = rectVertexShader;
      ShaderStages[0].pName = "main";

      ShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      ShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      ShaderStages[1].module = rectFragmentShader;
      ShaderStages[1].pName = "main";

      vtxBinding[0].binding = 0;
      vtxBinding[0].stride = sizeof(glm::vec2)*2;
      vtxBinding[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      vtxBinding[1].binding = 1;
      vtxBinding[1].stride = sizeof(glm::vec2)*2;
      vtxBinding[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

      vtxAttributes[0].binding = 0;
      vtxAttributes[0].location = 0;
      vtxAttributes[0].offset = 0;
      vtxAttributes[0].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[1].binding = 0;
      vtxAttributes[1].location = 1;
      vtxAttributes[1].offset = sizeof(glm::vec2);
      vtxAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[2].binding = 1;
      vtxAttributes[2].location = 2;
      vtxAttributes[2].offset = 0;
      vtxAttributes[2].format = VK_FORMAT_R32G32_SFLOAT;

      vtxAttributes[3].binding = 1;
      vtxAttributes[3].location = 3;
      vtxAttributes[3].offset = sizeof(glm::vec2);
      vtxAttributes[3].format = VK_FORMAT_R32G32_SFLOAT;

      InputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      InputState.vertexBindingDescriptionCount = 2;
      InputState.pVertexBindingDescriptions = vtxBinding;
      InputState.vertexAttributeDescriptionCount = 4;
      InputState.pVertexAttributeDescriptions = vtxAttributes;
    }

    VkGraphicsPipelineCreateInfo PipelineCI{};
    PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCI.renderPass = *Base.pRenderpass;
    PipelineCI.subpass = Base.Subpass;
    PipelineCI.layout = rectPipeLayout;
    PipelineCI.stageCount = 2;
    PipelineCI.pStages = ShaderStages;
    PipelineCI.pViewportState = &ViewStateInfo;
    PipelineCI.pInputAssemblyState = &InputAssemblyStateInfo;
    PipelineCI.pVertexInputState = &InputState;
    PipelineCI.pColorBlendState = &BlendStateInfo;
    PipelineCI.pMultisampleState = &MultiSampleStateInfo;
    PipelineCI.pDepthStencilState = &DepthStateInfo;
    PipelineCI.pRasterizationState = &RasterStateInfo;

    if((Err = vkCreateGraphicsPipelines(*Base.pDevice, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &rectPipe)) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create rect pipeline with error: " + std::to_string(Err));
    }

  }

  // Position and Size in pixels
  Widgets::Text* Renderer::CreateTextWidget(glm::vec2 Position, glm::vec2 Size, uint32_t EK_FONT_SIZE, const char* Value)
  {
    Widgets::Text Ret;
    Ret.strText = Value;
    Ret.pixPosition = Position;
    Ret.pixSize = Size;
    Ret.charDen = EK_FONT_SIZE;

    VkBufferCreateInfo txtBufferCI{};
    txtBufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    txtBufferCI.size = ( sizeof(glm::vec2) + sizeof(glm::vec2) + sizeof(uint32_t) ) * 2048;
    txtBufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    txtBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(*Base.pDevice, &txtBufferCI, nullptr, &Ret.instBuffer) != VK_SUCCESS)
    {
      throw std::runtime_error("UI Renderer: Failed to create Text widget buffer\n");
    }

    Tools::Allocate(*Base.pDevice, HostMemory, Ret.instBuffer, Ret.instMemory);

    TextWidgets.push_back(Ret);

    return &TextWidgets.back();
  }

  Widgets::Box* Renderer::CreateBoxWidget(glm::vec2 Position, glm::vec2 Size, glm::vec3 Color, float Smoothing)
  {
    Widgets::Box Ret;

    Ret.pixPosition = Position;
    Ret.pixSize = Size;
    Ret.Color = Color;
    Ret.Smoothing = Smoothing;

    // Allocate descriptor set
    {
      VkDescriptorSetAllocateInfo AllocInfo{};
      AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      AllocInfo.descriptorPool = BoxPool;
      AllocInfo.descriptorSetCount = 1;
      AllocInfo.pSetLayouts = &BoxLayout;

      if(vkAllocateDescriptorSets(*Base.pDevice, &AllocInfo, &Ret.Descriptor) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to allocate Box widget");
      }
    }

    // Create Uniform buffer on host visible memory
    {
      VkBufferCreateInfo BufferCI{};
      BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      BufferCI.size = sizeof(glm::vec3)+sizeof(float);
      BufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if(vkCreateBuffer(*Base.pDevice, &BufferCI, nullptr, &Ret.DescriptorBuffer) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to create UB for box widget");
      }

      VkMemoryRequirements MemReq{};
      vkGetBufferMemoryRequirements(*Base.pDevice, Ret.DescriptorBuffer, &MemReq);

      VkMemoryAllocateInfo MemAllocInfo{};
      MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      MemAllocInfo.allocationSize = MemReq.size;
      MemAllocInfo.memoryTypeIndex = HostMemory;

      if(vkAllocateMemory(*Base.pDevice, &MemAllocInfo, nullptr, &Ret.DescriptorMemory) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to allocate UB for box widget");
      }

      if(vkBindBufferMemory(*Base.pDevice, Ret.DescriptorBuffer, Ret.DescriptorMemory, 0) != VK_SUCCESS)
      {
        throw std::runtime_error("UI Renderer: Failed to bind UB for box widget");
      }
    }

    // Create Vertex Buffer
    {
      VkBufferCreateInfo BufferCI{};
      BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      BufferCI.size = sizeof(glm::vec2)*2;
      BufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if(vkCreateBuffer(*Base.pDevice, &BufferCI, nullptr, &Ret.instBuffer) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create instance buffer for box widget");
      }

      VkMemoryRequirements MemReq{};
      vkGetBufferMemoryRequirements(*Base.pDevice, Ret.instBuffer, &MemReq);

      VkMemoryAllocateInfo AllocInfo{};
      AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      AllocInfo.allocationSize = MemReq.size;
      AllocInfo.memoryTypeIndex = HostMemory;

      if(vkAllocateMemory(*Base.pDevice, &AllocInfo, nullptr, &Ret.instMemory) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate inst buffer");
      }

      vkBindBufferMemory(*Base.pDevice, Ret.instBuffer, Ret.instMemory, 0);
    }

    // Update Uniforms
    {
      float* Data;
      vkMapMemory(*Base.pDevice, Ret.DescriptorMemory, 0, sizeof(float)*4, 0, (void**)&Data);
        Data[0] = Color.x;
        Data[1] = Color.y;
        Data[2] = Color.z;
        Data[3] = Smoothing;
      vkUnmapMemory(*Base.pDevice, Ret.DescriptorMemory);

      VkDescriptorBufferInfo UniformWrite{};
      UniformWrite.range = sizeof(float)*4;
      UniformWrite.buffer = Ret.DescriptorBuffer;
      UniformWrite.offset = 0;

      VkWriteDescriptorSet WriteInfo{};
      WriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      WriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      WriteInfo.descriptorCount = 1;
      WriteInfo.pBufferInfo = &UniformWrite;
      WriteInfo.dstSet = Ret.Descriptor;
      WriteInfo.dstBinding = 0;

      vkUpdateDescriptorSets(*Base.pDevice, 1, &WriteInfo, 0, nullptr);
    }

    BoxWidgets.push_back(Ret);

    return &BoxWidgets.back();
  }

  void Renderer::UpdateWidgets()
  {
    // Update Text widgets
    for(uint32_t i = 0; i < TextWidgets.size(); i++)
    {
      struct {
        glm::vec2 Position;
        glm::vec2 Scale;
        uint32_t CharIndex;
      } *vtxInst;

      TextWidgets[i].charDrawCount = 0;

      if(TextWidgets[i].strText.size() != 0)
      {
        vkMapMemory(*Base.pDevice, TextWidgets[i].instMemory, 0, sizeof(*vtxInst)*TextWidgets[i].strText.size(), 0, (void**)&vtxInst);

        float Xpixel = 1.f/(float)WindowSize.width;
        float Ypixel = 1.f/(float)WindowSize.height;

        float penX = TextWidgets[i].pixPosition.x;
        float penY = TextWidgets[i].pixPosition.y;

        uint32_t instIdx = 0;

        // error: incrementing inst vertex index even when it's none-draw character ' '
          for(uint32_t x = 0; x < TextWidgets[i].strText.size(); x++)
          {
            char RenderChar = TextWidgets[i].strText[x];

            if(RenderChar == ' ')
            {
              penX += 20.f/(float)TextWidgets[i].charDen;
            }
            else
            {
              TextWidgets[i].charDrawCount++;

              glm::ivec2 ScaledPadding = {CharSet.Characters[RenderChar].pixPadding.x/TextWidgets[i].charDen, CharSet.Characters[RenderChar].pixPadding.y/TextWidgets[i].charDen};
              glm::ivec2 ScaledSize = {CharSet.Characters[RenderChar].pixSize.x/TextWidgets[i].charDen, CharSet.Characters[RenderChar].pixSize.y/TextWidgets[i].charDen};
              float ScaledOffset = CharSet.Characters[RenderChar].pixOffset/(float)TextWidgets[i].charDen;

              vtxInst[instIdx].Position.x = (penX+ScaledPadding.x)*Xpixel; 
              vtxInst[instIdx].Position.y = (penY-ScaledPadding.y)*Ypixel;

              vtxInst[instIdx].Scale.x = (ScaledSize.x)*Xpixel;
              vtxInst[instIdx].Scale.y = (ScaledSize.y)*Ypixel;

              vtxInst[instIdx].CharIndex = CharSet.Characters[RenderChar].DescriptorIndex;

              penX += ScaledOffset;

              instIdx++;
            }

            if(penX >= (TextWidgets[i].pixPosition.x + TextWidgets[i].pixSize.x))
            {
              if(penY > TextWidgets[i].pixSize.y)
              {
                break;
              }

              penY += 55.f;
              penX = TextWidgets[i].pixPosition.x;
            }
          }

        vkUnmapMemory(*Base.pDevice, TextWidgets[i].instMemory);
      }
    }

    // Update Box Widgets
    for(uint32_t i = 0; i < BoxWidgets.size(); i++)
    {
      struct {
        glm::vec2 Pos;
        glm::vec2 Size;
      } *inst;

      vkMapMemory(*Base.pDevice, BoxWidgets[i].instMemory, 0, sizeof(inst), 0, (void**)&inst);
        glm::vec2 Position = { BoxWidgets[i].pixPosition.x/WindowSize.width, BoxWidgets[i].pixPosition.y/WindowSize.height };
        glm::vec2 Size = { BoxWidgets[i].pixSize.x/WindowSize.width, BoxWidgets[i].pixSize.y/WindowSize.height };
        inst[i].Pos = Position;
        inst[i].Size = Size;
      vkUnmapMemory(*Base.pDevice, BoxWidgets[i].instMemory);
    }
  }

  void Renderer::DrawUI(VkCommandBuffer& cmdBuffer)
  {
    VkDeviceSize InstBufferOffset = 0;

    VkDeviceSize PlaneVtxOffset = offsetof(Primitives::PrimitiveBuffer, primPlane) + offsetof(Primitives::Plane, Vertices);
    VkDeviceSize PlaneIdxOffset = offsetof(Primitives::PrimitiveBuffer, primPlane) + offsetof(Primitives::Plane, Indices);

    VkDeviceSize TriVtxOffset = offsetof(Primitives::PrimitiveBuffer, primTriangle) + offsetof(Primitives::Triangle, Vertices);

    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, charPipeLayout, 0, 1, &FontDescriptor, 0, nullptr);
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, charPipe);

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &primBuffer, &PlaneVtxOffset);
    vkCmdBindIndexBuffer(cmdBuffer, primBuffer, PlaneIdxOffset, VK_INDEX_TYPE_UINT32);

    for(uint32_t i = 0; i < TextWidgets.size(); i++)
    {
      vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &TextWidgets[i].instBuffer, &InstBufferOffset);

      for(uint32_t x = 0; x < TextWidgets[i].charDrawCount; x++)
      {
        vkCmdDrawIndexed(cmdBuffer, 6, 1, 0, 0, x);
      }
    }
  }

  void Renderer::DrawRect(VkCommandBuffer& cmdBuffer)
  {
    VkDeviceSize instOffset = 0;
    VkDeviceSize vtxOffset = offsetof(Primitives::PrimitiveBuffer, primPlane) + offsetof(Primitives::Plane, Vertices);
    VkDeviceSize idxOffset = offsetof(Primitives::PrimitiveBuffer, primPlane) + offsetof(Primitives::Plane, Indices);
    
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rectPipe);
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &primBuffer, &vtxOffset);
    vkCmdBindIndexBuffer(cmdBuffer, primBuffer, idxOffset, VK_INDEX_TYPE_UINT32);

    for(uint32_t i = 0; i < BoxWidgets.size(); i++)
    {
      vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rectPipeLayout, 0, 1, &BoxWidgets[i].Descriptor, 0, 0);

      vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &BoxWidgets[i].instBuffer, &instOffset);
      
      vkCmdDrawIndexed(cmdBuffer, 6, 1, 0, 0, i);
    }
  }

  void Renderer::DrawTriangle(VkCommandBuffer& cmdBuffer)
  {
    VkDeviceSize PrimitiveOffset = offsetof(Primitives::PrimitiveBuffer, primTriangle)+offsetof(Primitives::Triangle, Vertices);

    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, curvePipe);

    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &primBuffer, &PrimitiveOffset);

    vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
  }
}

