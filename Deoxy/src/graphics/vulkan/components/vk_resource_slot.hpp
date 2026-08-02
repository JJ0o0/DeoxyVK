#pragma once

#include <optional>
#include <cstdint>

namespace deoxy::graphics::vulkan {
    template<typename T>
    struct ResourceSlot {
        std::optional<T> Resource;
        uint32_t Generation = 0;
    };
}
