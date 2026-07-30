#pragma once

#include <cstdint>
#include <limits>

namespace deoxy::graphics {
    class MeshHandle {
        public:
            static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

            uint32_t Index = InvalidIndex;
            uint32_t Generation = 0;

            constexpr bool IsValid() const noexcept { return Index != InvalidIndex; }
            constexpr explicit operator bool() const noexcept { return IsValid(); }
            constexpr bool operator==(const MeshHandle&) const noexcept = default;
    };
}
