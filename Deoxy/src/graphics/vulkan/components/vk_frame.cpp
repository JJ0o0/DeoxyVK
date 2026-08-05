#include "vk_frame.hpp"
#include "vk_command_pool.hpp"
#include "vk_allocator.hpp"
#include "vk_helper.hpp"
#include "vk_buffer.hpp"
#include "../shading/vk_uniforms.hpp"

#include <cstdint>

namespace deoxy::graphics::vulkan {
    VulkanFrame::VulkanFrame(VkDevice device, const VulkanCommandPool& commandPool, const VulkanAllocator& allocator)
        : m_device(device), m_commandPool(&commandPool) {
        CheckBool(m_device != VK_NULL_HANDLE, "VulkanFrame received a null logical device");

        try {
            // Criando o command buffer
            m_commandBuffer = m_commandPool->AllocatePrimary();

            // Criando um semáforo
            VkSemaphoreCreateInfo semaphoreCI {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
            };

            CheckResult(vkCreateSemaphore(m_device, &semaphoreCI, nullptr, &m_imageAvailableSemaphore));

            // Criando uma cerca
            VkFenceCreateInfo fenceCI {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT   // FENCE JÁ LIGADA! Se não: loop infinito no startup
            };

            CheckResult(vkCreateFence(m_device, &fenceCI, nullptr, &m_inFlightFence));

            // Criando buffer da camera
            m_cameraBuffer = std::make_unique<VulkanBuffer>(
                allocator,
                sizeof(FrameUniformData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_AUTO,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            );
        } catch (...) { destroy(); throw; }
    }

    VulkanFrame::~VulkanFrame() { destroy(); }

    void VulkanFrame::Wait() const {
        CheckResult(vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX));
    }

    void VulkanFrame::ResetForSubmit() {
        CheckResult(vkResetFences(m_device, 1, &m_inFlightFence));
        CheckResult(vkResetCommandBuffer(m_commandBuffer, 0));
    }

    void VulkanFrame::SetCameraDescriptorSet(VkDescriptorSet descriptorSet) {
        CheckBool(descriptorSet != VK_NULL_HANDLE, "Frame received a null camera descriptor set");
        m_cameraDescriptorSet = descriptorSet;
    }

    void VulkanFrame::destroy() {
        m_cameraBuffer.reset();

        if (m_inFlightFence != VK_NULL_HANDLE) vkDestroyFence(m_device, m_inFlightFence, nullptr);
        if (m_imageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
        if (m_commandBuffer != VK_NULL_HANDLE && m_commandPool != nullptr) {
            m_commandPool->Free(m_commandBuffer);
            m_commandBuffer = VK_NULL_HANDLE;
        }

        m_inFlightFence = VK_NULL_HANDLE;
        m_imageAvailableSemaphore = VK_NULL_HANDLE;
        m_cameraDescriptorSet = VK_NULL_HANDLE;
    }
}
