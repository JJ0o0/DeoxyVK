#pragma once

#include <deoxy/math/vec2.hpp>
#include <deoxy/math/vec3.hpp>
#include <deoxy/math/scalar.hpp>
#include <cmath>

namespace deoxy::math {
#pragma region Vec2
    inline float LengthSquared(const Vec2& vec) {
        return (vec.x * vec.x) + (vec.y * vec.y);
    }

    inline float Length(const Vec2& vec) {
        return sqrt(LengthSquared(vec));
    }

    inline Vec2 Normalize(const Vec2& vec) {
        const float lengthSquared = LengthSquared(vec);
        if (lengthSquared == 0) return Vec2{0.0f};

        float inverseLength = 1.0f / sqrt(lengthSquared);
        return vec * inverseLength;
    }

    inline float Dot(const Vec2& a, const Vec2& b) {
        return (a.x * b.x) + (a.y * b.y);
    }

    inline float DistanceSquared(const Vec2& a, const Vec2& b) {
        return LengthSquared(b - a);
    }

    inline float Distance(const Vec2& a, const Vec2& b) {
        return Length(b - a);
    }

    inline Vec2 Lerp(const Vec2& a, const Vec2& b, float t) {
        return (1.0f - t) * a + (t * b);
    }

    inline float Cross(const Vec2& a, const Vec2& b) {
        return (a.x * b.y) - (a.y * b.x);
    }

    inline Vec2 PerpendicularCCW(const Vec2& vec) {
        return Vec2 {-vec.y, vec.x};
    }

    inline Vec2 PerpendicularCW(const Vec2& vec) {
        return Vec2 {vec.y, -vec.x};
    }

    inline Vec2 Rotate(const Vec2& vec, float theta) {
        float thetaSin = Sin(theta);
        float thetaCos = Cos(theta);

        return Vec2 {
            (vec.x * thetaCos) - (vec.y * thetaSin),
            (vec.x * thetaSin) + (vec.y * thetaCos)
        };
    }
#pragma endregion Vec2

#pragma region Vec3
    inline float LengthSquared(const Vec3& vec) {
        return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
    }

    inline float Length(const Vec3& vec) {
        return sqrt(LengthSquared(vec));
    }

    inline Vec3 Normalize(const Vec3& vec) {
        const float lengthSquared = LengthSquared(vec);
        if (lengthSquared == 0) return Vec3{0.0f};

        float inverseLength = 1.0f / sqrt(lengthSquared);
        return vec * inverseLength;
    }

    inline float Dot(const Vec3& a, const Vec3& b) {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    inline float DistanceSquared(const Vec3& a, const Vec3& b) {
        return LengthSquared(b - a);
    }

    inline float Distance(const Vec3& a, const Vec3& b) {
        return Length(b - a);
    }

    inline Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
        return (1.0f - t) * a + (t * b);
    }

    inline Vec3 Cross(const Vec3& a, const Vec3& b) {
        return Vec3 {
            (a.y * b.z) - (a.z * b.y),
            (a.z * b.x) - (a.x * b.z),
            (a.x * b.y) - (a.y * b.x)
        };
    }
#pragma endregion Vec3
}
