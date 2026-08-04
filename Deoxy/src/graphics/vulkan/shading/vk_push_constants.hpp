#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/math.hpp>

#include <cstddef>
#include <type_traits>

namespace deoxy::graphics::vulkan {
    struct PushConstants {
        math::Mat4 ModelMatrix;
        Color MaterialTint{1.0f};
        math::Vec2 MaterialUVScale{1.0f};
        math::Vec2 MaterialUVOffset{0.0f};
    };

    static_assert(std::is_standard_layout_v<PushConstants>);

    static_assert(offsetof(PushConstants, ModelMatrix) == 0);
    static_assert(offsetof(PushConstants, MaterialTint) == 64);
    static_assert(offsetof(PushConstants, MaterialUVScale) == 80);
    static_assert(offsetof(PushConstants, MaterialUVOffset) == 88);
}
