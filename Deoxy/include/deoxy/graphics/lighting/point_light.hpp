#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

namespace deoxy::graphics {
    struct PointLight {
        math::Vec3 Position{0.0f};
        float Range = 5.0f;

        Color LightColor{1.0f};
        float Intensity = 1.0f;
    };
}
