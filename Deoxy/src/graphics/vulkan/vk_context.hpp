#pragma once

#include "components/vk_allocator.hpp"
#include "components/vk_command_pool.hpp"
#include "components/vk_device.hpp"
#include "components/vk_frame.hpp"
#include "components/vk_instance.hpp"
#include "components/vk_pipeline.hpp"
#include "components/vk_surface.hpp"
#include "components/vk_swapchain.hpp"
#include "components/vk_buffer.hpp"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <array>
#include <cstdint>

namespace deoxy::platform {
    class Window;
};

namespace deoxy::graphics {
    class VulkanContext {
        public:
            VulkanContext(platform::Window& window);
            ~VulkanContext();

            VulkanContext(const VulkanContext&) = delete;
            VulkanContext& operator=(const VulkanContext&) = delete;

            void DrawFrame();
        private:
            static constexpr uint32_t FRAMES_IN_FLIGHT = 2;

            void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

            // TEMPORÁRIO: Mover para classe Mesh
            void createGeometryBuffers();

            // ORDEM É IMPORTANTE!
            // Construção: Cima pra Baixo
            // Destruição: Baixo pra Cima

            vulkan::VulkanInstance m_instance;
            vulkan::VulkanSurface m_surface;
            vulkan::VulkanDevice m_device;
            vulkan::VulkanAllocator m_allocator;

            vulkan::VulkanCommandPool m_commandPool;
            vulkan::VulkanSwapchain m_swapchain;

            std::array<vulkan::VulkanFrame, FRAMES_IN_FLIGHT> m_frames;

            vulkan::VulkanPipeline m_pipeline;

            uint32_t m_currentFrame = 0;

            // TEMPORÁRIO: Mover para classe Mesh
            vulkan::VulkanBuffer m_vertexBuffer;
            vulkan::VulkanBuffer m_indexBuffer;

            uint32_t m_indexCount = 0;
    };
}
