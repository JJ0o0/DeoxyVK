#include <deoxy/platform/window.hpp>
#include <deoxy/platform/logger.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>

namespace deoxy::platform {
    struct Window::Impl {
        SDL_Window* Handle = nullptr;
        bool ShouldClose = false;
    };

    SDL_Window* Window::GetHandle() const { return m_impl->Handle; }

    Window::Window(const WindowProperties& properties)
        : m_impl(std::make_unique<Impl>()), m_properties(properties) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string{"Failed to initialize SDL: "} + SDL_GetError());
        }

        m_impl->Handle = SDL_CreateWindow(
            m_properties.Title.c_str(),
            static_cast<int>(m_properties.Width), static_cast<int>(m_properties.Height),
            SDL_WINDOW_VULKAN
        );

        if (!m_impl->Handle) {
            const std::string error = SDL_GetError();
            SDL_Quit();

            throw std::runtime_error{"Failed to create window: " + error};
        }

        Logger::Info("Created Window ({}x{})", m_properties.Width, m_properties.Height);
    }

    Window::~Window() { Destroy(); }

    void Window::Destroy() {
        if (m_impl->Handle) {
            SDL_DestroyWindow(m_impl->Handle);
            m_impl->Handle = nullptr;
        }

        SDL_Quit();
    }

    void Window::PollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                case SDL_EVENT_QUIT:
                    m_impl->ShouldClose = true;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    m_properties.Width = static_cast<uint32_t>(event.window.data1);
                    m_properties.Height = static_cast<uint32_t>(event.window.data2);
                    break;
                default: break;
            }
        }
    }

    bool Window::ShouldClose() const { return m_impl->ShouldClose; }

    const WindowProperties& Window::GetProperties() const { return m_properties; }
}
