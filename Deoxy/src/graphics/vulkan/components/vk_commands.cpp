#include "vk_commands.hpp"
#include "vk_command_pool.hpp"
#include "vk_helper.hpp"

namespace deoxy::graphics::vulkan {
    void CopyBufferImmediate(
        VkQueue queue,
        const VulkanCommandPool& commandPool,
        VkBuffer src, VkBuffer dest,
        VkDeviceSize size
    ) {
        CheckBool(queue != VK_NULL_HANDLE, "CopyBufferImmediate received a null queue");
        CheckBool(src != VK_NULL_HANDLE, "CopyBufferImmediate received a null source buffer");
        CheckBool(dest != VK_NULL_HANDLE, "CopyBufferImmediate received a null destination buffer");
        CheckBool(size > 0, "CopyBufferImmediate received a zero-sized copy");

        VkCommandBuffer commandBuffer = commandPool.AllocatePrimary();

        try {
            VkCommandBufferBeginInfo beginInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));
                VkBufferCopy copyRegion {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size = size
                };

                vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
            vulkan::CheckResult(vkEndCommandBuffer(commandBuffer));

            VkCommandBufferSubmitInfo commandBufferInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandBuffer,
                .deviceMask = 1
            };

            VkSubmitInfo2 submitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &commandBufferInfo
            };

            vulkan::CheckResult(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));
            vulkan::CheckResult(vkQueueWaitIdle(queue));
        } catch (...) {
            commandPool.Free(commandBuffer);
            throw;
        }

        commandPool.Free(commandBuffer);
    }

    void CopyBufferToImageImmediate(
        VkQueue queue,
        const VulkanCommandPool& commandPool,
        VkBuffer src, VkImage dest,
        uint32_t width, uint32_t height, uint32_t mipLevels
    ) {
        CheckBool(queue != VK_NULL_HANDLE, "CopyBufferToImageImmediate received a null queue");
        CheckBool(src != VK_NULL_HANDLE, "CopyBufferToImageImmediate received a null source buffer");
        CheckBool(dest != VK_NULL_HANDLE, "CopyBufferToImageImmediate received a null destination image");
        CheckBool(width > 0, "CopyBufferToImageImmediate received a zero-sized width");
        CheckBool(height > 0, "CopyBufferToImageImmediate received a zero-sized height");
        CheckBool(mipLevels > 0, "CopyBufferToImageImmediate received a zero mip levels");

        VkCommandBuffer commandBuffer = commandPool.AllocatePrimary();

        try {
            VkCommandBufferBeginInfo beginInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            };

            vulkan::CheckResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));
                TransitionImage(
                    commandBuffer, dest,
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_PIPELINE_STAGE_2_NONE, 0,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    0, mipLevels
                );

                VkBufferImageCopy copyRegion {
                    .bufferOffset = 0,
                    .bufferRowLength = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel = 0,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    },
                    .imageOffset = {0, 0, 0},
                    .imageExtent = {
                        .width = width,
                        .height = height,
                        .depth = 1
                    }
                };

                vkCmdCopyBufferToImage(commandBuffer, src, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                uint32_t mipWidth = width;
                uint32_t mipHeight = height;

                for (uint32_t mipLevel = 1; mipLevel < mipLevels; ++mipLevel) {
                    TransitionImage(
                        commandBuffer, dest,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        mipLevel - 1, 1
                    );

                    const uint32_t nextWidth = mipWidth > 1 ? mipWidth / 2 : 1;
                    const uint32_t nextHeight = mipHeight > 1 ? mipHeight / 2 : 1;
                    BlitImageMip(
                        commandBuffer, dest,
                        mipLevel - 1, mipLevel,
                        mipWidth, mipHeight,
                        nextWidth, nextHeight
                    );

                    TransitionImage(
                        commandBuffer, dest,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_READ_BIT,
                        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                        VK_IMAGE_ASPECT_COLOR_BIT,
                        mipLevel - 1, 1
                    );

                    mipWidth = nextWidth;
                    mipHeight = nextHeight;
                }

                TransitionImage(
                    commandBuffer, dest,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    mipLevels - 1, 1
                );
            vulkan::CheckResult(vkEndCommandBuffer(commandBuffer));

            VkCommandBufferSubmitInfo commandBufferInfo {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = commandBuffer,
                .deviceMask = 1
            };

            VkSubmitInfo2 submitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .commandBufferInfoCount = 1,
                .pCommandBufferInfos = &commandBufferInfo
            };

            vulkan::CheckResult(vkQueueSubmit2(queue, 1, &submitInfo, VK_NULL_HANDLE));
            vulkan::CheckResult(vkQueueWaitIdle(queue));
        } catch (...) {
            commandPool.Free(commandBuffer);
            throw;
        }

        commandPool.Free(commandBuffer);
    }

    void TransitionImage(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkPipelineStageFlags2 sourceStage,
        VkAccessFlags2 sourceAccess,
        VkPipelineStageFlags2 destinationStage,
        VkAccessFlags2 destinationAccess,
        VkImageAspectFlags aspectMask,
        uint32_t baseMipLevel,
        uint32_t levelCount
    ) {
        CheckBool(commandBuffer != VK_NULL_HANDLE, "TransitionImage received a null command buffer");
        CheckBool(image != VK_NULL_HANDLE, "TransitionImage received a null image");
        CheckBool(levelCount > 0, "TransitionImage received zero mip levels");

        VkImageMemoryBarrier2 barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = sourceStage,
            .srcAccessMask = sourceAccess,
            .dstStageMask = destinationStage,
            .dstAccessMask = destinationAccess,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange {
                .aspectMask = aspectMask,
                .baseMipLevel = baseMipLevel,
                .levelCount = levelCount,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkDependencyInfo dependencyInfo {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    }

    void BlitImageMip(
        VkCommandBuffer commandBuffer,
        VkImage image,
        uint32_t srcMipLevel, uint32_t destMipLevel,
        uint32_t srcWidth, uint32_t srcHeight,
        uint32_t destWidth, uint32_t destHeight
    ) {
        CheckBool(commandBuffer != VK_NULL_HANDLE, "BlitImageMip received a null command buffer");
        CheckBool(image != VK_NULL_HANDLE, "BlitImageMip received a null image");
        CheckBool(srcWidth > 0, "BlitImageMip received a invalid source width");
        CheckBool(srcHeight > 0, "BlitImageMip received a invalid source height");
        CheckBool(destWidth > 0, "BlitImageMip received a invalid destination width");
        CheckBool(destHeight > 0, "BlitImageMip received a invalid destination height");
        CheckBool(srcMipLevel != destMipLevel, "BlitImageMip received a source mip level that is equal to destination mip level");

        VkImageBlit2 imageBlit {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = srcMipLevel,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .srcOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{
                    static_cast<int32_t>(srcWidth),
                    static_cast<int32_t>(srcHeight),
                    1
                }
            },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = destMipLevel,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .dstOffsets = {
                VkOffset3D{0, 0, 0},
                VkOffset3D{
                    static_cast<int32_t>(destWidth),
                    static_cast<int32_t>(destHeight),
                    1
                }
            },
        };

        VkBlitImageInfo2 blitImageInfo {
            .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
            .srcImage = image,
            .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .dstImage = image,
            .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .regionCount = 1,
            .pRegions = &imageBlit,
            .filter = VK_FILTER_LINEAR
        };

        vkCmdBlitImage2(commandBuffer, &blitImageInfo);
    }
}
