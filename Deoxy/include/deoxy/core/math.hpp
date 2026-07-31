#pragma once

// VAZANDO GLM PRA API! TEMPORÁRIO
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <cmath>
#include <numbers>

namespace deoxy::core::math {
    inline constexpr float PI = std::numbers::pi_v<float>;
    inline constexpr float TWO_PI = PI * 2.0f;
    inline constexpr float HALF_PI = PI * 0.5f;

    // VAZANDO GLM PRA API! TEMPORÁRIO
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Mat4 = glm::mat4;

    constexpr float ToRadians(float degrees) { return (degrees * PI) / 180.0f; }
    constexpr float ToDegrees(float degrees) { return (degrees * 180.0f) / PI; }

    inline float Sin(float radians) { return std::sin(radians); }
    inline float Cos(float radians) { return std::cos(radians); }
    inline float Tan(float radians) { return std::tan(radians); }

    inline float Asin(float value) { return std::asin(value); }
    inline float Acos(float value) { return std::acos(value); }
    inline float Atan(float value) { return std::atan(value); }
    inline float Atan2(float y, float x) { return std::atan2(y, x); }
}
