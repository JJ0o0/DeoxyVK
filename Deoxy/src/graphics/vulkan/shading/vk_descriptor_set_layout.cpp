#include "vk_descriptor_set_layout.hpp"
#include "../components/vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDevice device, std::span<const VkDescriptorSetLayoutBinding> bindings)
        : m_device(device) {
        CheckBool(device != VK_NULL_HANDLE, "DescriptorSetLayout received a null logical device");

        VkDescriptorSetLayoutCreateInfo descriptorCI {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.empty() ? nullptr : bindings.data()
        };

        CheckResult(vkCreateDescriptorSetLayout(device, &descriptorCI, nullptr, &m_handle));
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() { destroy(); }

    void VulkanDescriptorSetLayout::destroy() {
        if (m_handle != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, m_handle, nullptr);
    }
}
