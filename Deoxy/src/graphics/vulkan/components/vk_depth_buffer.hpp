#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

namespace deoxy::graphics::vulkan {
    class VulkanDepthBuffer {
        public:
            VulkanDepthBuffer(
                VkDevice device,
                VmaAllocator allocator,
                VkExtent2D extent,
                VkFormat depthFormat
            );

            ~VulkanDepthBuffer();

            VulkanDepthBuffer(const VulkanDepthBuffer&) = delete;
            VulkanDepthBuffer& operator=(const VulkanDepthBuffer&) = delete;

            VulkanDepthBuffer(VulkanDepthBuffer&& other) noexcept;
            VulkanDepthBuffer& operator=(VulkanDepthBuffer&& other) noexcept;

            VkImage GetImage() const { return m_image; }
            VkImageView GetImageView() const { return m_imageView; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;
            VmaAllocator m_allocator = nullptr;

            void destroy();

            VkImage m_image = VK_NULL_HANDLE;
            VkImageView m_imageView = VK_NULL_HANDLE;
            VmaAllocation m_allocation = nullptr;
    };
}
