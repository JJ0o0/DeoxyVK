#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

namespace deoxy::graphics {
    struct Vertex {
        math::Vec3 Position{0.0f};
        Color Tint{1.0f};
    };
}
