#pragma once

#include <deoxy/math/constants.hpp>
#include <deoxy/math/scalar.hpp>
#include <deoxy/math/mat4.hpp>
#include <cassert>

namespace deoxy::math {
    inline Mat4 Perspective(float fovyRadians, float aspectRatio, float nearPlane, float farPlane) {
        assert(fovyRadians > 0.0f);
        assert(fovyRadians < constants::PI);
        assert(aspectRatio > 0.0f);
        assert(nearPlane > 0.0f);
        assert(farPlane > nearPlane);

        const float scale = 1.0f / Tan(fovyRadians * 0.5f);

        Mat4 result{};
        result(0, 0) = scale / aspectRatio;
        result(1, 1) = scale;
        result(2, 2) = farPlane / (nearPlane - farPlane);
        result(2, 3) = (farPlane * nearPlane) / (nearPlane - farPlane);
        result(3, 2) = -1.0f;

        return result;
    }
}
