#pragma once

#include <memory>
#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanCommandPool;
    class VulkanAllocator;
    class VulkanBuffer;
    class VulkanFrame {
        public:
            VulkanFrame(VkDevice device, const VulkanCommandPool& commandPool, const VulkanAllocator& allocator);
            ~VulkanFrame();

            VulkanFrame(const VulkanFrame&) = delete;
            VulkanFrame& operator=(const VulkanFrame&) = delete;

            VulkanFrame(VulkanFrame&&) = delete;
            VulkanFrame& operator=(VulkanFrame&&) = delete;

            void Wait() const;

            // SOMENTE APÓS O ACQUIRE FUNCIONAR!
            void ResetForSubmit();

            VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

            VulkanBuffer& GetCameraBuffer() { return *m_cameraBuffer; }
            const VulkanBuffer& GetCameraBuffer() const { return *m_cameraBuffer; }

            VkDescriptorSet GetCameraDescriptorSet() const { return m_cameraDescriptorSet; }
            void SetCameraDescriptorSet(VkDescriptorSet descriptorSet);

            VkSemaphore GetImageAvailableSemaphore() const { return m_imageAvailableSemaphore; }
            VkFence GetFence() const { return m_inFlightFence; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            const VulkanCommandPool* m_commandPool = nullptr;

            void destroy();

            std::unique_ptr<VulkanBuffer> m_cameraBuffer;
            VkDescriptorSet m_cameraDescriptorSet = VK_NULL_HANDLE;
            VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
            VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
            VkFence m_inFlightFence = VK_NULL_HANDLE;
    };
}
