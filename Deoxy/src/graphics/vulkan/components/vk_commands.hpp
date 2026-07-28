#pragma once

#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanCommandPool;

    // Cria, envia e espera a operação inteira de copiar buffer
    void CopyBufferImmediate(
        VkQueue queue,
        const VulkanCommandPool& commandPool,
        VkBuffer src, VkBuffer dest,
        VkDeviceSize size
    );

    // Grava um comando no command buffer
    void TransitionImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags2 sourceStage,
        VkAccessFlags2 sourceAccess,
        VkPipelineStageFlags2 destinationStage,
        VkAccessFlags2 destinationAccess,
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
    );
}
