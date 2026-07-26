#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace deoxy::platform {
    struct WindowProperties {
        std::string Title = "DeoxyVK";
        uint32_t Width = 800;
        uint32_t Height = 600;
    };

    class Window {
        public:
            Window(const WindowProperties& properties = {});
            ~Window();

            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;

            void Destroy();
            void PollEvents();

            bool ShouldClose() const;

            const WindowProperties& GetProperties() const;
        private:
            struct Impl;
            std::unique_ptr<Impl> m_impl;

            WindowProperties m_properties;
    };
}
