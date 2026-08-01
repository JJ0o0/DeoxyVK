#pragma once

#include <deoxy/math/operations.hpp>
#include <deoxy/math/vec3.hpp>
#include <deoxy/math/mat4.hpp>

namespace deoxy::math {
    inline Mat4 Translate(const Mat4& matrix, const Vec3& position) {
        Mat4 translationMatrix{1.0f};
        translationMatrix(0, 3) = position.x;
        translationMatrix(1, 3) = position.y;
        translationMatrix(2, 3) = position.z;

        return matrix * translationMatrix;
    }

    inline Mat4 RotateX(const Mat4& matrix, float radians) {
        float thetaCos = Cos(radians);
        float thetaSin = Sin(radians);

        Mat4 rotationMatrix{1.0f};
        rotationMatrix(1, 1) = thetaCos;
        rotationMatrix(1, 2) = -thetaSin;
        rotationMatrix(2, 1) = thetaSin;
        rotationMatrix(2, 2) = thetaCos;

        return matrix * rotationMatrix;
    }

    inline Mat4 RotateY(const Mat4& matrix, float radians) {
        float thetaCos = Cos(radians);
        float thetaSin = Sin(radians);

        Mat4 rotationMatrix{1.0f};
        rotationMatrix(0, 0) = thetaCos;
        rotationMatrix(0, 2) = thetaSin;
        rotationMatrix(2, 0) = -thetaSin;
        rotationMatrix(2, 2) = thetaCos;

        return matrix * rotationMatrix;
    }

    inline Mat4 RotateZ(const Mat4& matrix, float radians) {
        float thetaCos = Cos(radians);
        float thetaSin = Sin(radians);

        Mat4 rotationMatrix{1.0f};
        rotationMatrix(0, 0) = thetaCos;
        rotationMatrix(0, 1) = -thetaSin;
        rotationMatrix(1, 0) = thetaSin;
        rotationMatrix(1, 1) = thetaCos;

        return matrix * rotationMatrix;
    }

    inline Mat4 Scale(const Mat4& matrix, const Vec3& scale) {
        Mat4 scaleMatrix{};
        scaleMatrix(0, 0) = scale.x;
        scaleMatrix(1, 1) = scale.y;
        scaleMatrix(2, 2) = scale.z;
        scaleMatrix(3, 3) = 1.0f;

        return matrix * scaleMatrix;
    }
}
