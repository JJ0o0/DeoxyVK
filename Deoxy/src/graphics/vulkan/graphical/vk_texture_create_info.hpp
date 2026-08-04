#pragma once

#include <volk.h>

namespace deoxy::graphics::vulkan {
    struct VulkanTextureCreateInfo {
        VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
        VkFilter Filter = VK_FILTER_LINEAR;
        VkSamplerMipmapMode MipmapFilter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        VkSamplerAddressMode WrapMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        bool GenerateMipmaps = true;

        bool AnisotropyEnabled = false;
        float MaxAnisotropy = 1.0f;
    };
}
