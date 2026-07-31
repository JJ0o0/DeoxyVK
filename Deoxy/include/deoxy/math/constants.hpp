#pragma once

#include <numbers>

namespace deoxy::math::constants {
    inline constexpr float PI = std::numbers::pi_v<float>;
    inline constexpr float TWO_PI = PI * 2.0f;
    inline constexpr float HALF_PI = PI * 0.5f;
}
