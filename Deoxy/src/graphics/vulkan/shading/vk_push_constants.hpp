#pragma once

#include <deoxy/math/math.hpp>

namespace deoxy::graphics::vulkan {
    struct MeshPushConstants {
        math::Mat4 ModelMatrix;
    };
}
