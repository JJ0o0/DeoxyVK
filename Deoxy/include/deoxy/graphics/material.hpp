#pragma once

#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec2.hpp>

namespace deoxy::graphics {
    struct MaterialCreateInfo {
        TextureHandle Albedo;
        Color Tint{1.0f};
        math::Vec2 UVScale{1.0f};
        math::Vec2 UvOffset{0.0f};
    };
}
