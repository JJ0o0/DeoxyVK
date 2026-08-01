#pragma once

#include <deoxy/math/mat4.hpp>
#include <volk.h>
#include <array>

namespace deoxy::graphics::vulkan {
    struct CameraUniformData {
        math::Mat4 View{1.0f};
        math::Mat4 Projection{1.0f};
    };

    inline std::array<VkDescriptorSetLayoutBinding, 1> CameraBindings {
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    inline std::array<VkDescriptorSetLayoutBinding, 1> TextureBindings {
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };

    inline std::array<VkDescriptorPoolSize, 1> CreateCameraPoolSizes(uint32_t frameCount) {
        return {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = frameCount
            }
        };
    }

    inline std::array<VkDescriptorPoolSize, 1> CreateTexturePoolSizes(uint32_t textureCount) {
        return {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = textureCount
            }
        };
    }
}
