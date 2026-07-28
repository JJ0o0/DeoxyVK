#pragma once

#include <volk.h>
#include <format>
#include <string>

namespace deoxy::graphics::vulkan {
    static inline void CheckResult(VkResult result) {
        if (result != VK_SUCCESS) {
            std::string msg = std::format("Vulkan call returned an error ({})", static_cast<int>(result));
            throw std::runtime_error{msg};
        }
    }

    static inline void CheckBool(bool result, const std::string& errorMessage) {
        if (!result) throw std::runtime_error{errorMessage};
    }
}
