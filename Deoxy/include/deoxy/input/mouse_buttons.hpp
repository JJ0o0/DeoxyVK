#pragma once

#include <cstdint>

namespace deoxy::input {
    enum class Mouse : uint16_t {
        Left, Middle, Right,
        Side1, Side2,
        Count
    };
}
