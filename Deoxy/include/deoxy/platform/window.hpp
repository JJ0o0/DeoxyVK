#pragma once

#include <deoxy/input/input.hpp>
#include <filesystem>
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
        bool Resizable = false;
    };

    class Window {
        public:
            Window(const WindowProperties& properties = {});
            ~Window();

            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;

            void Destroy();
            void PollEvents(Input& input);

            bool ShouldClose() const;

            void SetTitle(std::string_view title);
            void SetIcon(const std::filesystem::path& path);
            void SetFullscreen(bool fullscreen);
            void SetRelativeMouseMode(bool enabled);

            bool IsFullscreen() const;

            float GetAspectRatio() const { return static_cast<float>(m_properties.Width) / static_cast<float>(m_properties.Height); }
            const WindowProperties& GetProperties() const;
        private:
            friend class deoxy::graphics::VulkanContext;

            SDL_Window* GetHandle() const;

            struct Impl;
            std::unique_ptr<Impl> m_impl;

            WindowProperties m_properties;
    };
}
