#include "vk_mesh.hpp"
#include "../components/vk_commands.hpp"
#include "../components/vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanMesh::VulkanMesh(
        const VulkanAllocator& allocator, const VulkanCommandPool& commandPool,
        VkQueue queue,
        std::span<const Vertex> vertices, std::span<const std::uint32_t> indices
    )
        : m_vertexBuffer(
            allocator,
            vertices.size_bytes(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            0
        ), m_indexBuffer(
            allocator,
            indices.size_bytes(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            0
        ), m_indexCount(static_cast<std::uint32_t>(indices.size())) {
            CheckBool(!vertices.empty(), "Cannot create a mesh without vertices");
            CheckBool(!indices.empty(), "Cannot create a mesh without indices");

            // Cria o staging buffer para o vertex
            const VkDeviceSize vertexBufferSize = vertices.size_bytes();
            VulkanBuffer stagingVertexBuffer {
                allocator, vertexBufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            };

            // Copia os dados da CPU
            stagingVertexBuffer.Upload(vertices.data(), vertexBufferSize);

            // Copia staging para a GPU
            CopyBufferImmediate(
                queue,
                commandPool,
                stagingVertexBuffer.GetHandle(),
                m_vertexBuffer.GetHandle(),
                vertexBufferSize
            );

            // Cria o staging buffer para o index
            const VkDeviceSize indexBufferSize = indices.size_bytes();
            VulkanBuffer stagingIndexBuffer {
                allocator, indexBufferSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            };

            // Copia os dados da CPU
            stagingIndexBuffer.Upload(indices.data(), indexBufferSize);

            // Copia staging para a GPU
            CopyBufferImmediate(
                queue,
                commandPool,
                stagingIndexBuffer.GetHandle(),
                m_indexBuffer.GetHandle(),
                indexBufferSize
            );
    }

    void VulkanMesh::Bind(VkCommandBuffer commandBuffer) const {
        const VkBuffer vertexBuffer = m_vertexBuffer.GetHandle();
        const VkDeviceSize vertexOffset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexOffset);
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer.GetHandle(), 0, VK_INDEX_TYPE_UINT32);
    }

    void VulkanMesh::Draw(VkCommandBuffer commandBuffer) const {
        Bind(commandBuffer);
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
    }
}
