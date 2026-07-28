#pragma once

#include <algorithm>
#include <cstdint>

namespace deoxy::graphics {
    struct Color {
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
        float A = 1.0f;
    };

    struct Color32 {
        uint8_t R = 255;
        uint8_t G = 255;
        uint8_t B = 255;
        uint8_t A = 255;
    };

    constexpr Color ToColor(const Color32& color32) {
        return Color {
            .R = static_cast<float>(color32.R) / 255.0f,
            .G = static_cast<float>(color32.G) / 255.0f,
            .B = static_cast<float>(color32.B) / 255.0f,
            .A = static_cast<float>(color32.A) / 255.0f,
        };
    }

    constexpr Color32 ToColor32(const Color& color) {
        const auto toByte = [](float channel) constexpr -> uint8_t {
            channel = std::clamp(channel, 0.0f, 1.0f);
            return static_cast<uint8_t>((channel * 255.0f) + 0.5f);
        };

        return Color32 {
            .R = toByte(color.R),
            .G = toByte(color.G),
            .B = toByte(color.B),
            .A = toByte(color.A),
        };
    }
}
