#pragma once

#include <string_view>
#include <utility>
#include <format>

namespace deoxy::platform {
    class Logger {
        public:
            static void Initialize();

            static void Info(std::string_view msg);
            static void Warn(std::string_view msg);
            static void Error(std::string_view msg);

            template <typename... Args>
            static void Info(std::format_string<Args...> format, Args &&...args) {
                Info(std::format(format, std::forward<Args>(args)...));
            }

            template <typename... Args>
            static void Warn(std::format_string<Args...> format, Args &&...args) {
                Warn(std::format(format, std::forward<Args>(args)...));
            }

            template <typename... Args>
            static void Error(std::format_string<Args...> format, Args &&...args) {
                Error(std::format(format, std::forward<Args>(args)...));
            }
    };
}
