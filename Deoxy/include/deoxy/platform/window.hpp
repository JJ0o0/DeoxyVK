#pragma once

#include <cstdint>
#include <memory>
#include <string>

struct SDL_Window;
namespace deoxy::graphics {
    class VulkanContext;
}

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
            friend class deoxy::graphics::VulkanContext;

            SDL_Window* GetHandle() const;

            struct Impl;
            std::unique_ptr<Impl> m_impl;

            WindowProperties m_properties;
    };
}
