#pragma once

#include <deoxy/graphics/image_data.hpp>

namespace deoxy::graphics {
    enum class TextureColorSpace {
        SRGB,
        Linear
    };

    enum class TextureFilter {
        Linear,
        Nearest
    };

    enum class TextureWrapMode {
        Repeat,
        MirroredRepeat,
        ClampToEdge
    };

    enum class TextureMipmapFilter {
        Linear,
        Nearest
    };

    struct TextureCreateInfo {
        TextureColorSpace ColorSpace = TextureColorSpace::SRGB;
        TextureFilter Filter = TextureFilter::Linear;
        TextureMipmapFilter MipmapFilter = TextureMipmapFilter::Linear;
        TextureWrapMode WrapMode = TextureWrapMode::Repeat;

        bool GenerateMipmaps = true;

        bool EnableAnisotropy = true;
        float AnisotropyLevel = 16.0f;
    };
}
