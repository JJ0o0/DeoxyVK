#pragma once

namespace deoxy::math {
    struct Vec2 {
        float x = 0.0f;
        float y = 0.0f;

        constexpr Vec2() = default;
        constexpr Vec2(float xValue, float yValue) : x(xValue), y(yValue) {}
        explicit constexpr Vec2(float value) : x(value), y(value) {}

        Vec2& operator+=(const Vec2& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2& operator-=(const Vec2& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2& operator*=(const Vec2& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        Vec2& operator*=(float scalar) {
            x *= scalar;
            y *= scalar;
            return *this;
        }
    };

    inline Vec2 operator+(Vec2 left, const Vec2& right) {
        left += right;
        return left;
    }

    inline Vec2 operator-(Vec2 left, const Vec2& right) {
        left -= right;
        return left;
    }

    inline Vec2 operator*(Vec2 left, const Vec2& right) {
        left *= right;
        return left;
    }

    inline Vec2 operator*(Vec2 vector, float scalar) {
        vector.x *= scalar;
        vector.y *= scalar;
        return vector;
    }

    inline Vec2 operator*(float scalar, Vec2 vector) {
        vector.x *= scalar;
        vector.y *= scalar;
        return vector;
    }

    inline Vec2 operator-(const Vec2& vector) {
        return {-vector.x, -vector.y};
    }
}
