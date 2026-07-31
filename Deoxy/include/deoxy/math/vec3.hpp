#pragma once

namespace deoxy::math {
    struct Vec3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;

        constexpr Vec3() = default;
        constexpr Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}
        explicit constexpr Vec3(float value) : x(value), y(value), z(value) {}

        Vec3& operator+=(const Vec3& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        Vec3& operator-=(const Vec3& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        Vec3& operator*=(const Vec3& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        Vec3& operator*=(float scalar) {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }
    };

    inline Vec3 operator+(Vec3 left, const Vec3& right) {
        left += right;
        return left;
    }

    inline Vec3 operator-(Vec3 left, const Vec3& right) {
        left -= right;
        return left;
    }

    inline Vec3 operator*(Vec3 left, const Vec3& right) {
        left *= right;
        return left;
    }

    inline Vec3 operator*(Vec3 vector, float scalar) {
        vector.x *= scalar;
        vector.y *= scalar;
        vector.z *= scalar;
        return vector;
    }

    inline Vec3 operator*(float scalar, Vec3 vector) {
        vector.x *= scalar;
        vector.y *= scalar;
        vector.z *= scalar;
        return vector;
    }

    inline Vec3 operator-(const Vec3& vector) {
        return {-vector.x, -vector.y, -vector.z};
    }
}
