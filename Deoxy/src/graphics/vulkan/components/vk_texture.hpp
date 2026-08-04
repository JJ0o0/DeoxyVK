#pragma once

#include "vk_image.hpp"
#include "vk_sampler.hpp"

#include <deoxy/graphics/image_data.hpp>

#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanAllocator;
    class VulkanCommandPool;
    class VulkanTexture {
    public:
        VulkanTexture(
            VkDevice device,
            const VulkanAllocator& allocator,
            const VulkanCommandPool& commandPool,
            VkQueue queue,
            const ImageData& imageData
        );

        ~VulkanTexture() = default;

        VulkanTexture(const VulkanTexture&) = delete;
        VulkanTexture& operator=(const VulkanTexture&) = delete;

        VulkanTexture(VulkanTexture&&) noexcept = default;
        VulkanTexture& operator=(VulkanTexture&&) noexcept = default;

        const VulkanImage& GetImage() const { return m_image; }
        VkImageView GetImageView() const { return m_image.GetImageView(); }
        VkSampler GetSampler() const { return m_sampler.GetHandle(); }
    private:
        uint32_t m_mipLevels;
        VulkanImage m_image;
        VulkanSampler m_sampler;

        uint32_t calculateMipLevels(uint32_t width, uint32_t height);
    };
}
