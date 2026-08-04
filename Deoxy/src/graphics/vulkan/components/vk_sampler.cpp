#include "vk_sampler.hpp"
#include "vk_helper.hpp"

#include <utility>

namespace deoxy::graphics::vulkan {
    VulkanSampler::VulkanSampler(
        VkDevice device,
        uint32_t mipLevels,
        VkFilter filter, VkSamplerMipmapMode mipmapFilter,
        VkSamplerAddressMode wrapMode,
        bool anisotropyEnabled, float maxAnisotropy
    ) : m_device(device) {
        CheckBool(m_device != VK_NULL_HANDLE, "Sampler received a null logical device");
        CheckBool(mipLevels > 0, "Sampler received invalid mip levels");
        CheckBool(filter == VK_FILTER_LINEAR || filter == VK_FILTER_NEAREST, "Sampler received an unsupported texture filter");
        CheckBool(mipmapFilter == VK_SAMPLER_MIPMAP_MODE_LINEAR || mipmapFilter == VK_SAMPLER_MIPMAP_MODE_NEAREST, "Sampler received an unsupported texture mipmap filter");
        CheckBool(
            wrapMode == VK_SAMPLER_ADDRESS_MODE_REPEAT ||
            wrapMode == VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT ||
            wrapMode == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            "Sampler received an unsupported wrap mode"
        );
        CheckBool(maxAnisotropy >= 1.0f, "Sampler received invalid maximum anisotropy");

        VkSamplerCreateInfo samplerCI {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = filter,
            .minFilter = filter,
            .mipmapMode = mipmapFilter,
            .addressModeU = wrapMode,
            .addressModeV = wrapMode,
            .addressModeW = wrapMode,
            .mipLodBias = 0,
            .anisotropyEnable = anisotropyEnabled ? VK_TRUE : VK_FALSE,
            .maxAnisotropy = anisotropyEnabled ? maxAnisotropy : 1.0f,
            .compareEnable = VK_FALSE,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(mipLevels - 1),
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
