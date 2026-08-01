#include <deoxy/graphics/image_loader.hpp>
#include <stb_image.h>
#include <cstdint>
#include <memory>
#include <format>

namespace deoxy::graphics {
    ImageData ImageLoader::Load(const std::filesystem::path& path) {
        const std::string pathString = path.string();
        int width = 0, height = 0;

        using STBIData = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
        STBIData data {
            stbi_load(pathString.c_str(), &width, &height, nullptr, STBI_rgb_alpha),
            &stbi_image_free
        };

        if (!data) {
            throw std::runtime_error(
                std::format("Failed to load texture at {}: {}", pathString, stbi_failure_reason())
            );
        }

        if (width <= 0 || height <= 0) {
            throw std::runtime_error(
                std::format("Failed to load texture at {}: invalid dimensions", pathString)
            );
        }

        ImageData result {};
        result.Width = static_cast<uint32_t>(width);
        result.Height = static_cast<uint32_t>(height);

        const size_t byteCount = static_cast<size_t>(width) * static_cast<size_t>(height) * STBI_rgb_alpha;
        result.Pixels.assign(data.get(), data.get() + byteCount);

        return result;
    }
}
