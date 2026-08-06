#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>
#include <deoxy/math/mat4.hpp>
#include <cstddef>
#include <volk.h>
#include <array>

namespace deoxy::graphics::vulkan {
    struct AmbientLightUniformData {
        math::Vec3 LightColor{1.0f};
        float Intensity = 0.15f;
    };

    struct DirectionalLightUniformData {
        math::Vec3 Direction{0.0f, -1.0f, 0.0f};
        float Intensity = 0.0f;
        Color LightColor{1.0f};
    };

    struct PointLightUniformData {
        math::Vec3 Position{0.0f, 0.0f, 0.0f};
        float Range = 0.0f;

        math::Vec3 LightColor{1.0f};
        float Intensity = 0.0f;
    };

    struct SpotLightUniformData {
        math::Vec3 Position{0.0f};
        float Range = 0.0f;

        math::Vec3 Direction{0.0f, -1.0f, 0.0f};
        float InnerAngleCos = 20.0f;

        math::Vec3 LightColor{1.0f};
        float Intensity = 0.0f;

        float OuterAngleCos = 30.0f;
        std::array<float, 3> Padding{};
    };

    inline constexpr size_t MAX_POINT_LIGHTS = 16;
    inline constexpr size_t MAX_SPOT_LIGHTS = 8;

    struct FrameUniformData {
        math::Mat4 View{1.0f};
        math::Mat4 Projection{1.0f};
        AmbientLightUniformData AmbientLight{};
        DirectionalLightUniformData DirectionalLight{};

        uint32_t PointLightCount = 0;
        alignas(16) std::array<PointLightUniformData, MAX_POINT_LIGHTS> PointLights{};

        uint32_t SpotLightCount = 0;
        alignas(16) std::array<SpotLightUniformData, MAX_SPOT_LIGHTS> SpotLights{};
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
