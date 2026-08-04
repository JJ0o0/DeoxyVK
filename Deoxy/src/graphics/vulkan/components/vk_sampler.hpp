#pragma once

#include <volk.h>

namespace deoxy::graphics::vulkan {
    class VulkanSampler {
        public:
            VulkanSampler() = default;

            explicit VulkanSampler(
                VkDevice device,
                uint32_t mipLevels,
                VkFilter filter, VkSamplerMipmapMode mipmapFilter,
                VkSamplerAddressMode wrapMode,
                bool anisotropyEnabled, float maxAnisotropy
            );

            ~VulkanSampler();

            VulkanSampler(const VulkanSampler&) = delete;
            VulkanSampler& operator=(const VulkanSampler&) = delete;

            VulkanSampler(VulkanSampler&& other) noexcept;
            VulkanSampler& operator=(VulkanSampler&& other) noexcept;

            VkSampler GetHandle() const { return m_sampler; }
        private:
            VkDevice m_device = VK_NULL_HANDLE;

            void destroy();

            VkSampler m_sampler = VK_NULL_HANDLE;
    };
}
