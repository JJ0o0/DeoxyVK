#pragma once

#include <deoxy/graphics/image_data.hpp>
#include <filesystem>

namespace deoxy::graphics {
    class ImageLoader {
        public:
            static ImageData Load(const std::filesystem::path& path);
    };
}
