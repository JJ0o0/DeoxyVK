#pragma once

#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanCommandPool;
    class VulkanFrame {
        public:
            VulkanFrame(VkDevice device, const VulkanCommandPool& commandPool);
            ~VulkanFrame();

            VulkanFrame(const VulkanFrame&) = delete;
            VulkanFrame& operator=(const VulkanFrame&) = delete;

            VulkanFrame(VulkanFrame&&) = delete;
            VulkanFrame& operator=(VulkanFrame&&) = delete;

            void Wait() const;

            // SOMENTE APÓS O ACQUIRE FUNCIONAR!
            void ResetForSubmit();

            VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }
            VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
            VkFence GetFence() const { return m_inFlightFence; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            const VulkanCommandPool* m_commandPool = nullptr;

            void destroy();

            VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
            VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
            VkFence m_inFlightFence = VK_NULL_HANDLE;
    };
}
