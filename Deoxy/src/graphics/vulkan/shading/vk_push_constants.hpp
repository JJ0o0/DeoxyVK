#pragma once

#include <deoxy/core/math.hpp>

namespace deoxy::graphics::vulkan {
    struct MeshPushConstants {
        core::math::Mat4 ModelMatrix;
    };
}
