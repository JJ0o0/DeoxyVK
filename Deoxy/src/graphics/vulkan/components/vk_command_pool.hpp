#pragma once

#include <volk.h>

#include <cstdint>
#include <vector>

namespace deoxy::graphics::vulkan {
    class VulkanCommandPool {
        public:
            VulkanCommandPool(VkDevice device, uint32_t graphicsQueueFamily);
            ~VulkanCommandPool();

            VulkanCommandPool(const VulkanCommandPool&) = delete;
            VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;

            VkCommandBuffer AllocatePrimary() const;
            std::vector<VkCommandBuffer> AllocatePrimary(uint32_t count) const;
            void Free(VkCommandBuffer commandBuffer) const;

            VkCommandPool GetHandle() const { return m_commandPool; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;

            VkCommandPool m_commandPool = VK_NULL_HANDLE;
    };
}
