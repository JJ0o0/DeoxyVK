#pragma once

#include <volk.h>
#include <cstdint>

namespace deoxy::graphics::vulkan {
    class VulkanCommandPool;

    // Cria, envia e espera a operação inteira de copiar buffer
    void CopyBufferImmediate(
        VkQueue queue,
        const VulkanCommandPool& commandPool,
        VkBuffer src, VkBuffer dest,
        VkDeviceSize size
    );

    // Mesma coisa de CopyBufferImmediate, só que para Image
    void CopyBufferToImageImmediate(
        VkQueue queue,
        const VulkanCommandPool& commandPool,
        VkBuffer src, VkImage dest,
        uint32_t width, uint32_t height, uint32_t mipLevels
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
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 1
    );

    // Copiar um mip para o seguinte
    void BlitImageMip(
        VkCommandBuffer commandBuffer,
        VkImage image,
        uint32_t srcMipLevel, uint32_t destMipLevel,
        uint32_t srcWidth, uint32_t srcHeight,
        uint32_t destWidth, uint32_t destHeight
    );
}
