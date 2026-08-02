#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/math.hpp>

namespace deoxy::graphics::vulkan {
    struct PushConstants {
        math::Mat4 ModelMatrix;
        Color MaterialTint{1.0f};
    };
}
