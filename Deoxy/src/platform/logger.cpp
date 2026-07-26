#include <deoxy/platform/logger.hpp>
#include <spdlog/spdlog.h>

namespace deoxy::platform {
    void Logger::Initialize() {
        spdlog::set_pattern("%^[%H:%M:%S] [%l] %v%$");
        spdlog::set_level(spdlog::level::debug);
    }

    void Logger::Info(std::string_view message) { spdlog::info("{}", message); }
    void Logger::Warn(std::string_view message) { spdlog::warn("{}", message); }
    void Logger::Error(std::string_view message) { spdlog::error("{}", message); }
}
