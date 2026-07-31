#pragma once

#include <cassert>
#include <cstddef>

namespace deoxy::math {
    struct Mat4 {
        float Data[16]{};

        constexpr Mat4() = default;

        explicit constexpr Mat4(float diagonal) : Data{} {
            Data[0] = diagonal;
            Data[5] = diagonal;
            Data[10] = diagonal;
            Data[15] = diagonal;
        }

        float& operator()(size_t row, size_t column) {
            assert(row < 4);
            assert(column < 4);

            return Data[(column * 4) + row];
        }

        const float& operator()(size_t row, size_t column) const {
            assert(row < 4);
            assert(column < 4);

            return Data[(column * 4) + row];
        }

        Mat4& operator*=(const Mat4& other) {
            Mat4 result{};

            for (size_t row = 0; row < 4; row++) {
                for (size_t column = 0; column < 4; column++) {
                    for (size_t k = 0; k < 4; k++) {
                        result(row, column) += (*this)(row, k) * other(k, column);
                    }
                }
            }

            *this = result;
            return *this;
        }
    };

    inline Mat4 operator*(Mat4 left, const Mat4& right) {
        left *= right;
        return left;
    }
}
