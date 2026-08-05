#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>
#include <deoxy/math/mat4.hpp>
#include <volk.h>
#include <array>

namespace deoxy::graphics::vulkan {
    struct DirectionalLightUniformData {
        math::Vec3 Direction{0.0f, -1.0f, 0.0f};
        float Intensity = 1.0f;
        Color LightColor{1.0f};
    };

    struct FrameUniformData {
        math::Mat4 View{1.0f};
        math::Mat4 Projection{1.0f};
        DirectionalLightUniformData DirectionalLight;
    };

    inline std::array<VkDescriptorSetLayoutBinding, 1> CameraBindings {
        VkDescriptorSetLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
