#include "vk_image.hpp"
#include "vk_helper.hpp"

#include <utility>

namespace deoxy::graphics::vulkan {
    VulkanImage::VulkanImage(
        VkDevice device,
        VmaAllocator allocator,
        VkExtent2D extent,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectMask
    ) : m_device(device), m_allocator(allocator),
        m_format(format), m_extent(extent) {
        CheckBool(m_device != VK_NULL_HANDLE, "VulkanImage received a null logical device");
        CheckBool(m_allocator != nullptr, "VulkanImage received a null allocator");
        CheckBool(extent.width > 0 && extent.height > 0, "VulkanImage received a zero-sized extent");
        CheckBool(format != VK_FORMAT_UNDEFINED, "VulkanImage received an undefined format");
        CheckBool(usage != 0, "VulkanImage received an undefined usage");
        CheckBool(aspectMask != 0, "VulkanImage received an undefined aspect mask");

        VkImageCreateInfo imageCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_format,
            .extent = {
                .width = m_extent.width,
                .height = m_extent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocationCI {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };

        CheckResult(vmaCreateImage(m_allocator, &imageCI, &allocationCI, &m_image, &m_allocation, nullptr));

        try {
            VkImageViewCreateInfo viewCI {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_format,
                .subresourceRange = {
                    .aspectMask = aspectMask,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            CheckResult(vkCreateImageView(m_device, &viewCI, nullptr, &m_imageView));
        } catch (...) { destroy(); throw; }
    }

    VulkanImage::~VulkanImage() { destroy(); }

    VulkanImage::VulkanImage(VulkanImage&& other) noexcept
        : m_device(std::exchange(other.m_device, VK_NULL_HANDLE)),
          m_allocator(std::exchange(other.m_allocator, nullptr)),
          m_format(std::exchange(other.m_format, VK_FORMAT_UNDEFINED)),
          m_extent(std::exchange(other.m_extent, VkExtent2D{})),
          m_image(std::exchange(other.m_image, VK_NULL_HANDLE)),
          m_imageView(std::exchange(other.m_imageView, VK_NULL_HANDLE)),
          m_allocation(std::exchange(other.m_allocation, nullptr)) {
    }

    VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept {
        if (this == &other) return *this;

        destroy();

        m_device = std::exchange(other.m_device, VK_NULL_HANDLE);
        m_allocator = std::exchange(other.m_allocator, nullptr);
        m_format = std::exchange(other.m_format, VK_FORMAT_UNDEFINED);
        m_extent = std::exchange(other.m_extent, VkExtent2D{});
        m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
        m_imageView = std::exchange(other.m_imageView, VK_NULL_HANDLE);
        m_allocation = std::exchange(other.m_allocation, nullptr);

        return *this;
    }

    void VulkanImage::destroy() {
        if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE) vmaDestroyImage(m_allocator, m_image, m_allocation);

        m_imageView = VK_NULL_HANDLE;
        m_image = VK_NULL_HANDLE;
        m_allocation = nullptr;
        m_device = VK_NULL_HANDLE;
        m_allocator = nullptr;
        m_format = VK_FORMAT_UNDEFINED;
        m_extent = VkExtent2D{};
    }
}
