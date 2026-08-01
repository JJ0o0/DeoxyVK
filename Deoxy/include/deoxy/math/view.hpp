#pragma once

#include <deoxy/math/operations.hpp>
#include <deoxy/math/vec3.hpp>
#include <deoxy/math/mat4.hpp>
#include <cassert>

namespace deoxy::math {
    inline Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        assert(LengthSquared(target - eye) > 0.0f);
        assert(LengthSquared(up) > 0.0f);

        Vec3 forward = Normalize(target - eye);
        assert(LengthSquared(Cross(forward, up)) > 0.0f);

        Vec3 right = Normalize(Cross(forward, up));
        Vec3 cameraUp = Cross(right, forward);

        Mat4 result{1.0f};

        result(0, 0) = right.x;
        result(0, 1) = right.y;
        result(0, 2) = right.z;
        result(0, 3) = -Dot(right, eye);

        result(1, 0) = cameraUp.x;
        result(1, 1) = cameraUp.y;
        result(1, 2) = cameraUp.z;
        result(1, 3) = -Dot(cameraUp, eye);

        result(2, 0) = -forward.x;
        result(2, 1) = -forward.y;
        result(2, 2) = -forward.z;
        result(2, 3) = Dot(forward, eye);

        result(3, 3) = 1.0f;

        return result;
    }
}
