#pragma once

#include <volk.h>
#include <vector>
#include <span>

namespace deoxy::graphics::vulkan {
    class VulkanDescriptorPool {
        public:
            VulkanDescriptorPool(VkDevice device, std::span<const VkDescriptorPoolSize> poolSizes, uint32_t maxSets);
            ~VulkanDescriptorPool();

            VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
            VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;

            VulkanDescriptorPool(VulkanDescriptorPool&&) = delete;
            VulkanDescriptorPool& operator=(VulkanDescriptorPool&&) = delete;

            std::vector<VkDescriptorSet> Allocate(std::span<const VkDescriptorSetLayout> layouts) const;

            VkDescriptorPool GetHandle() const { return m_handle; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;

            void destroy();

            VkDescriptorPool m_handle = VK_NULL_HANDLE;
    };
}
