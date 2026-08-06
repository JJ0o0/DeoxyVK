#include "vk_push_constants.hpp"
#include "vk_uniforms.hpp"

#include <deoxy/graphics/vertex.hpp>

#include <cstddef>
#include <type_traits>

#define DEOXY_VK_VALIDATE_GPU_STRUCT(Type, ExpectedSize) \
    static_assert(std::is_standard_layout_v<Type>, #Type " must be standard-layout"); \
    static_assert(std::is_trivially_copyable_v<Type>, #Type " must be trivially copyable"); \
    static_assert(sizeof(Type) == ExpectedSize, #Type " has an unexpected size");

#define DEOXY_VK_VALIDATE_GPU_MEMBER(Type, Member, ExpectedOffset) \
    static_assert(offsetof(Type, Member) == ExpectedOffset, #Type "::" #Member " has an unexpected offset")

namespace deoxy::graphics::vulkan {
    DEOXY_VK_VALIDATE_GPU_STRUCT(PushConstants, 96);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PushConstants, ModelMatrix, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PushConstants, MaterialTint, 64);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PushConstants, MaterialUVScale, 80);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PushConstants, MaterialUVOffset, 88);

    DEOXY_VK_VALIDATE_GPU_STRUCT(FrameUniformData, 704);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, View, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, Projection, 64);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, AmbientLight, 128);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, DirectionalLight, 144);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, PointLightCount, 176);
    DEOXY_VK_VALIDATE_GPU_MEMBER(FrameUniformData, PointLights, 192);

    DEOXY_VK_VALIDATE_GPU_STRUCT(AmbientLightUniformData, 16);
    DEOXY_VK_VALIDATE_GPU_MEMBER(AmbientLightUniformData, LightColor, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(AmbientLightUniformData, Intensity, 12);

    DEOXY_VK_VALIDATE_GPU_STRUCT(DirectionalLightUniformData, 32);
    DEOXY_VK_VALIDATE_GPU_MEMBER(DirectionalLightUniformData, Direction, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(DirectionalLightUniformData, Intensity, 12);
    DEOXY_VK_VALIDATE_GPU_MEMBER(DirectionalLightUniformData, LightColor, 16);

    DEOXY_VK_VALIDATE_GPU_STRUCT(PointLightUniformData, 32);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PointLightUniformData, Position, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PointLightUniformData, Range, 12);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PointLightUniformData, LightColor, 16);
    DEOXY_VK_VALIDATE_GPU_MEMBER(PointLightUniformData, Intensity, 28);
}

namespace deoxy::graphics {
    DEOXY_VK_VALIDATE_GPU_STRUCT(Vertex, 48);
    DEOXY_VK_VALIDATE_GPU_MEMBER(Vertex, Position, 0);
    DEOXY_VK_VALIDATE_GPU_MEMBER(Vertex, Tint, 12);
    DEOXY_VK_VALIDATE_GPU_MEMBER(Vertex, UV, 28);
    DEOXY_VK_VALIDATE_GPU_MEMBER(Vertex, Normal, 36);
}

#undef DEOXY_VK_VALIDATE_GPU_MEMBER
#undef DEOXY_VK_VALIDATE_GPU_STRUCT
