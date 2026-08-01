#pragma once

#include <cstdint>
#include <vector>

namespace deoxy::graphics {
    struct ImageData {
        std::vector<uint8_t> Pixels;

        uint32_t Width = 0;
        uint32_t Height = 0;
    };
}
