#pragma once

#include "../components/vk_allocator.hpp"
#include "../components/vk_command_pool.hpp"
#include "../components/vk_buffer.hpp"

#include <deoxy/graphics/vertex.hpp>

#include <cstdint>
#include <volk.h>
#include <span>

namespace deoxy::graphics::vulkan {
    class VulkanMesh {
        public:
            VulkanMesh(
                const VulkanAllocator& allocator, const VulkanCommandPool& commandPool,
                VkQueue queue,
                std::span<const Vertex> vertices, std::span<const std::uint32_t> indices
            );

            VulkanMesh(const VulkanMesh&) = delete;
            VulkanMesh& operator=(const VulkanMesh&) = delete;

            VulkanMesh(VulkanMesh&&) noexcept = default;
            VulkanMesh& operator=(VulkanMesh&&) noexcept = default;

            void Bind(VkCommandBuffer commandBuffer) const;
            void Draw(VkCommandBuffer commandBuffer) const;
        private:
            VulkanBuffer m_vertexBuffer;
            VulkanBuffer m_indexBuffer;

            uint32_t m_indexCount = 0;
    };
}
