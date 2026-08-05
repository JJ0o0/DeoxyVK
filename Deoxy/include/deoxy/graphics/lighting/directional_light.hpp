#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

namespace deoxy::graphics {
    struct DirectionalLight {
        math::Vec3 Direction{0.0f, -1.0f, 0.0f};
        float Intensity = 1.0f;
        Color LightColor{1.0f};
    };
}
