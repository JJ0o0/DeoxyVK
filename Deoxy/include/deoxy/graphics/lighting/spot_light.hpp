#pragma once

#include <deoxy/graphics/color.hpp>
#include <deoxy/math/vec3.hpp>

namespace deoxy::graphics {
    struct SpotLight {
        math::Vec3 Position{0.0f};
        float Range = 5.0f;

        math::Vec3 Direction{0.0f, -1.0f, 0.0f};
        float InnerAngle = 20.0f;
        float OuterAngle = 30.0f;

        Color LightColor{1.0f};
        float Intensity = 1.0f;
    };
}
