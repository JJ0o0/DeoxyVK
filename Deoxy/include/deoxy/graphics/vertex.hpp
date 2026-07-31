#pragma once

#include <deoxy/math/math.hpp>

namespace deoxy::graphics {
    struct Vertex {
        math::Vec3 Position{0.0f};
        math::Vec3 Color{1.0f};
    };
}
