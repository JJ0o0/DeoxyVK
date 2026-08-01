#include "vk_depth_buffer.hpp"
#include "vk_helper.hpp"

#include <utility>

namespace deoxy::graphics::vulkan {
    VulkanDepthBuffer::VulkanDepthBuffer(
        VkDevice device,
        VmaAllocator allocator,
        VkExtent2D extent,
        VkFormat depthFormat
    ) : m_device(device), m_allocator(allocator) {
        CheckBool(m_device != VK_NULL_HANDLE, "DepthBuffer received a null logical device");
        CheckBool(m_allocator != nullptr, "DepthBuffer received a null allocator");
        CheckBool(extent.width > 0 && extent.height > 0, "DepthBuffer received a zero-sized extent");
        CheckBool(depthFormat != VK_FORMAT_UNDEFINED, "DepthBuffer received an undefined format");

        // Alocar e criar a imagem de depth
        VkImageCreateInfo depthImageCI {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = depthFormat,
            .extent {
                .width = extent.width, .height = extent.height,
                .depth = 1
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        VmaAllocationCreateInfo allocCI {
            .usage = VMA_MEMORY_USAGE_AUTO
        };

        CheckResult(vmaCreateImage(m_allocator, &depthImageCI, &allocCI, &m_image, &m_allocation, nullptr));

        try {
            // Criando image view do depth
            VkImageViewCreateInfo depthViewCI {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_image,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = depthFormat,
                .subresourceRange {
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                }
            };

            CheckResult(vkCreateImageView(m_device, &depthViewCI, nullptr, &m_imageView));
        } catch (...) { destroy(); throw; }
    }

    VulkanDepthBuffer::VulkanDepthBuffer(VulkanDepthBuffer&& other) noexcept
        : m_device(std::exchange(other.m_device, VK_NULL_HANDLE)),
          m_allocator(std::exchange(other.m_allocator, nullptr)),
          m_image(std::exchange(other.m_image, VK_NULL_HANDLE)),
          m_imageView(std::exchange(other.m_imageView, VK_NULL_HANDLE)),
          m_allocation(std::exchange(other.m_allocation, nullptr)) {
    }

    VulkanDepthBuffer& VulkanDepthBuffer::operator=(VulkanDepthBuffer&& other) noexcept {
        if (this == &other) return *this;

        destroy();

        m_device = std::exchange(other.m_device, VK_NULL_HANDLE);
        m_allocator = std::exchange(other.m_allocator, nullptr);
        m_image = std::exchange(other.m_image, VK_NULL_HANDLE);
        m_imageView = std::exchange(other.m_imageView, VK_NULL_HANDLE);
        m_allocation = std::exchange(other.m_allocation, nullptr);

        return *this;
    }

    VulkanDepthBuffer::~VulkanDepthBuffer() { destroy(); }

    void VulkanDepthBuffer::destroy() {
        if (m_imageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_imageView, nullptr);
        if (m_image != VK_NULL_HANDLE) vmaDestroyImage(m_allocator, m_image, m_allocation);

        m_imageView = VK_NULL_HANDLE;
        m_image = VK_NULL_HANDLE;
        m_allocation = nullptr;
        m_device = VK_NULL_HANDLE;
        m_allocator = nullptr;
    }
}
