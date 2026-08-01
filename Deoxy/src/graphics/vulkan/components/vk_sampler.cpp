#include "vk_sampler.hpp"
#include "vk_helper.hpp"

#include <utility>

namespace deoxy::graphics::vulkan {
    VulkanSampler::VulkanSampler(VkDevice device)
        : m_device(device) {
        CheckBool(m_device != VK_NULL_HANDLE, "Sampler received a null logical device");

        VkSamplerCreateInfo samplerCI {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_FALSE,
            .compareEnable = VK_FALSE,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        CheckResult(vkCreateSampler(m_device, &samplerCI, nullptr, &m_sampler));
    }

    VulkanSampler::VulkanSampler(VulkanSampler&& other) noexcept
        : m_device(std::exchange(other.m_device, VK_NULL_HANDLE)),
          m_sampler(std::exchange(other.m_sampler, VK_NULL_HANDLE)) {
    }

    VulkanSampler& VulkanSampler::operator=(VulkanSampler&& other) noexcept {
        if (this == &other) return *this;

        destroy();

        m_device = std::exchange(other.m_device, VK_NULL_HANDLE);
        m_sampler = std::exchange(other.m_sampler, VK_NULL_HANDLE);

        return *this;
    }

    VulkanSampler::~VulkanSampler() { destroy(); }

    void VulkanSampler::destroy() {
        if (m_sampler != VK_NULL_HANDLE) vkDestroySampler(m_device, m_sampler, nullptr);

        m_sampler = VK_NULL_HANDLE;
        m_device = VK_NULL_HANDLE;
    }
}
