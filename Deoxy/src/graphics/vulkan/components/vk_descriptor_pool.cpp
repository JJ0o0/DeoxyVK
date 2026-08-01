#include "vk_descriptor_pool.hpp"
#include "vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, std::span<const VkDescriptorPoolSize> poolSizes, uint32_t maxSets)
        : m_device(device) {
        CheckBool(device != VK_NULL_HANDLE, "DescriptorSetLayout received a null logical device");

        VkDescriptorPoolCreateInfo poolCI {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = 0,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        CheckResult(vkCreateDescriptorPool(device, &poolCI, nullptr, &m_handle));
    }

    VulkanDescriptorPool::~VulkanDescriptorPool() { destroy(); }

    std::vector<VkDescriptorSet> VulkanDescriptorPool::Allocate(std::span<const VkDescriptorSetLayout> layouts) const {
        CheckBool(!layouts.empty(), "Cannot allocate zero descriptor sets");

        std::vector<VkDescriptorSet> descriptorSets(layouts.size());

        VkDescriptorSetAllocateInfo allocateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_handle,
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
        };

        CheckResult(vkAllocateDescriptorSets(m_device, &allocateInfo, descriptorSets.data()));
        return descriptorSets;
    }

    void VulkanDescriptorPool::destroy() {
        if (m_handle != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, m_handle, nullptr);
    }
}
