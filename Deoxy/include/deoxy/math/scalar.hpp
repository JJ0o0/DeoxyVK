#pragma once

#include <deoxy/math/constants.hpp>

#include <cmath>

namespace deoxy::math {
    constexpr float ToRadians(float degrees) { return (degrees * constants::PI) / 180.0f; }
    constexpr float ToDegrees(float radians) { return (radians * 180.0f) / constants::PI; }

    inline float Sin(float radians) { return std::sin(radians); }
    inline float Cos(float radians) { return std::cos(radians); }
    inline float Tan(float radians) { return std::tan(radians); }

    inline float Asin(float value) { return std::asin(value); }
    inline float Acos(float value) { return std::acos(value); }
    inline float Atan(float value) { return std::atan(value); }
    inline float Atan2(float y, float x) { return std::atan2(y, x); }
}
