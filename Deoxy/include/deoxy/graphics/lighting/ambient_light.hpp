#pragma once

#include <deoxy/graphics/color.hpp>

namespace deoxy::graphics {
    struct AmbientLight {
        float Intensity = 0.15f;
        Color LightColor{1.0f};
    };
}
