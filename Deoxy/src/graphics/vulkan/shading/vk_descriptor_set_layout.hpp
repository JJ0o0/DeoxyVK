#pragma once

#include <volk.h>
#include <span>

namespace deoxy::graphics::vulkan {
    class VulkanDescriptorSetLayout {
        public:
            VulkanDescriptorSetLayout(VkDevice device, std::span<const VkDescriptorSetLayoutBinding> bindings);
            ~VulkanDescriptorSetLayout();

            VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
            VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

            VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&&) = delete;
            VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&&) = delete;

            VkDescriptorSetLayout GetHandle() { return m_handle; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;

            void destroy();

            VkDescriptorSetLayout m_handle = VK_NULL_HANDLE;
    };
}
