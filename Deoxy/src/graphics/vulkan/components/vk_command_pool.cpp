#include "vk_command_pool.hpp"
#include "vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    VulkanCommandPool::VulkanCommandPool(VkDevice device, uint32_t graphicsQueueFamily)
        : m_device(device) {
        // Verificando se recebemos um ponteiro vazio de device
        CheckBool(device != VK_NULL_HANDLE, "Command Pool received a null logical device");

        // Criando a Command Pool usando a graphics queue family
        VkCommandPoolCreateInfo poolCI {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphicsQueueFamily
        };

        CheckResult(vkCreateCommandPool(m_device, &poolCI, nullptr, &m_commandPool));
    }

    VulkanCommandPool::~VulkanCommandPool() {
        if (m_commandPool != VK_NULL_HANDLE) vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }

    VkCommandBuffer VulkanCommandPool::AllocatePrimary() const {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkCommandBufferAllocateInfo allocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        CheckResult(vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer));
        return commandBuffer;
    }

    std::vector<VkCommandBuffer> VulkanCommandPool::AllocatePrimary(uint32_t count) const {
        CheckBool(count > 0, "Cannot allocate zero command buffers");

        std::vector<VkCommandBuffer> commandBuffers(count, VK_NULL_HANDLE);
        VkCommandBufferAllocateInfo allocInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count
        };

        CheckResult(vkAllocateCommandBuffers(m_device, &allocInfo, commandBuffers.data()));
        return commandBuffers;
    }

    void VulkanCommandPool::Free(VkCommandBuffer commandBuffer) const {
        if (commandBuffer == VK_NULL_HANDLE) return;
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }
}
