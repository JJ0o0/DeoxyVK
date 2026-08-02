#pragma once

#include <deoxy/graphics/graphical_handles.hpp>
#include <deoxy/graphics/color.hpp>

namespace deoxy::graphics {
    struct MaterialCreateInfo {
        TextureHandle Albedo;
        Color Tint{1.0f};
    };
}
