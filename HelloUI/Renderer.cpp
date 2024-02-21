#include "Renderer.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace Rendering
{
  UnlitRenderer::UnlitRenderer(RendererCreateInfo& CI)
  {
    Instance = CI.Instance;
    PhysicalDevice = CI.PhysicalDevice;
    Device = CI.Device;
    GraphicsQueue = CI.GraphicsQueue;
    RenderPass = CI.RenderPass;
    HostMemory = CI.HostMemory;
    LocalMemory = CI.LocalMemory;
    SubpassIndex = CI.SubpassIndex;
  }

  void UnlitRenderer::LoadMesh(const char* MeshPath)
  {
    Mesh newMesh;

    Assimp::Importer Importer;

    const aiScene* Scene = Importer.ReadFile(MeshPath, aiProcess_Triangulate);

    for(uint32_t i = 0; i < Scene->mNumMeshes; i++)
    {
      aiMesh* pMesh = Scene->mMeshes[i];

      for(uint32_t vtx = 0; vtx < Scene->mMeshes[i]->mNumVertices; vtx++)
      {
        Vertex tmpVtx;
        tmpVtx.Position = {pMesh->mVertices[vtx].x, pMesh->mVertices[vtx].y, pMesh->mVertices[vtx].z};
        tmpVtx.UV.x = pMesh->mTextureCoords[0][vtx].x;
        tmpVtx.UV.y = pMesh->mTextureCoords[0][vtx].y;

        newMesh.Vertices.push_back(tmpVtx);
      }

      for(uint32_t faceIdx = 0; faceIdx < Scene->mMeshes[i]->mNumFaces; faceIdx++)
      {
        newMesh.Indices.push_back(Scene->mMeshes[i]->mFaces[faceIdx].mIndices[0]);
        newMesh.Indices.push_back(Scene->mMeshes[i]->mFaces[faceIdx].mIndices[1]);
        newMesh.Indices.push_back(Scene->mMeshes[i]->mFaces[faceIdx].mIndices[2]);
      }

      VkBufferCreateInfo BufferCI{};
      BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      BufferCI.size =  ( sizeof(Vertex)*newMesh.Vertices.size() ) * ( sizeof(uint32_t)*newMesh.Indices.size() );
      BufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if(vkCreateBuffer(Device, &BufferCI, nullptr, &newMesh.Buffer) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to create buffer for mesh\n");
      }

      VkMemoryRequirements MemReq{};
      vkGetBufferMemoryRequirements(Device, newMesh.Buffer, &MemReq);

      VkMemoryAllocateInfo AllocInfo{};
      AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      AllocInfo.allocationSize = MemReq.size;
      AllocInfo.memoryTypeIndex = HostMemory;

      if(vkAllocateMemory(Device, &AllocInfo, nullptr, &newMesh.Memory) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to allocate buffer for mesh");
      }

      if(vkBindBufferMemory(Device, newMesh.Buffer, newMesh.Memory, 0) != VK_SUCCESS)
      {
        throw std::runtime_error("Failed to bind buffer for mesh");
      }

      Meshes.push_back(newMesh);
    }
  }

  void UnlitRenderer::Draw()
  {
  }
}
