#include "vk_texture.hpp"

#include "vk_allocator.hpp"
#include "vk_buffer.hpp"
#include "vk_command_pool.hpp"
#include "../components/vk_commands.hpp"
#include "../components/vk_helper.hpp"

#include <deoxy/math/scalar.hpp>

namespace deoxy::graphics::vulkan {
    VulkanTexture::VulkanTexture(
        VkDevice device,
        const VulkanAllocator& allocator,
        const VulkanCommandPool& commandPool,
        VkQueue queue,
        const ImageData& imageData,
        const VulkanTextureCreateInfo& createInfo
    ) : m_mipLevels(
            createInfo.GenerateMipmaps
            ? calculateMipLevels(imageData.Width, imageData.Height)
            : 1
        ),
        m_image(
            device, allocator.GetHandle(),
            VkExtent2D{
                .width = imageData.Width,
                .height = imageData.Height
            },
            createInfo.Format,
            getTextureUsage(createInfo.GenerateMipmaps),
            VK_IMAGE_ASPECT_COLOR_BIT,
            m_mipLevels
        ), m_sampler(
            device,
            m_mipLevels,
            createInfo.Filter, createInfo.MipmapFilter,
            createInfo.WrapMode,
            createInfo.AnisotropyEnabled, createInfo.MaxAnisotropy
        ) {
        CheckBool(queue != VK_NULL_HANDLE, "Texture received a null queue");
        CheckBool(createInfo.Format != VK_FORMAT_UNDEFINED, "Texture received an undefined format");
        CheckBool(!imageData.Pixels.empty(), "Texture received empty pixel data");

        const size_t expectedSize = static_cast<size_t>(imageData.Width) * static_cast<size_t>(imageData.Height) * 4;
        CheckBool(imageData.Pixels.size() == expectedSize, "Texture received an invalid RGBA pixel count");

        const VkDeviceSize imageSize = static_cast<VkDeviceSize>(imageData.Pixels.size());
        VulkanBuffer stagingBuffer {
            allocator, imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        };

        stagingBuffer.Upload(imageData.Pixels.data(), imageData.Pixels.size());

        CopyBufferToImageImmediate(
            queue, commandPool, stagingBuffer.GetHandle(), m_image.GetImage(),
            imageData.Width, imageData.Height, m_mipLevels
        );
    }

    uint32_t VulkanTexture::calculateMipLevels(uint32_t width, uint32_t height) {
        CheckBool(width > 0, "Texture received invalid width for mip levels calculation");
        CheckBool(height > 0, "Texture received invalid height for mip levels calculation");

        return math::Floor(
            math::Log2(
                static_cast<float>(math::Max(width, height))
            )
        ) + 1;
    }

    VkImageUsageFlags VulkanTexture::getTextureUsage(bool generateMipmaps) {
        VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        if (generateMipmaps) usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        return usage;
    }
}
