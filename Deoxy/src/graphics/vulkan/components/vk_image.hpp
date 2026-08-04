#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

namespace deoxy::graphics::vulkan {
    class VulkanImage {
        public:
            VulkanImage() = default;
            VulkanImage(
                VkDevice device,
                VmaAllocator allocator,
                VkExtent2D extent,
                VkFormat format,
                VkImageUsageFlags usage,
                VkImageAspectFlags aspectMask,
                uint32_t mipLevels
            );

            ~VulkanImage();

            VulkanImage(const VulkanImage&) = delete;
            VulkanImage& operator=(const VulkanImage&) = delete;

            VulkanImage(VulkanImage&& other) noexcept;
            VulkanImage& operator=(VulkanImage&& other) noexcept;

            VkImage GetImage() const { return m_image; }
            VkImageView GetImageView() const { return m_imageView; }
            VkFormat GetFormat() const { return m_format; }
            VkExtent2D GetExtent() const { return m_extent; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VmaAllocator m_allocator = nullptr;
            VkFormat m_format = VK_FORMAT_UNDEFINED;
            VkExtent2D m_extent{};

            void destroy();

            VkImage m_image = VK_NULL_HANDLE;
            VkImageView m_imageView = VK_NULL_HANDLE;
            VmaAllocation m_allocation = nullptr;
    };
}
