#include <deoxy/platform/window.hpp>
#include <deoxy/platform/logger.hpp>
#include <deoxy/input/key.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>

namespace deoxy::platform {
    struct Window::Impl {
        SDL_Window* Handle = nullptr;
        bool ShouldClose = false;
    };

    SDL_Window* Window::GetHandle() const { return m_impl->Handle; }

    static std::optional<input::Key> ToDeoxyKey(SDL_Scancode scancode) {
        using enum input::Key;

        switch (scancode) {
            case SDL_SCANCODE_Q: return Q;
            case SDL_SCANCODE_W: return W;
            case SDL_SCANCODE_E: return E;
            case SDL_SCANCODE_R: return R;
            case SDL_SCANCODE_T: return T;
            case SDL_SCANCODE_Y: return Y;
            case SDL_SCANCODE_U: return U;
            case SDL_SCANCODE_I: return I;
            case SDL_SCANCODE_O: return O;
            case SDL_SCANCODE_P: return P;
            case SDL_SCANCODE_A: return A;
            case SDL_SCANCODE_S: return S;
            case SDL_SCANCODE_D: return D;
            case SDL_SCANCODE_F: return F;
            case SDL_SCANCODE_G: return G;
            case SDL_SCANCODE_H: return H;
            case SDL_SCANCODE_J: return J;
            case SDL_SCANCODE_K: return K;
            case SDL_SCANCODE_L: return L;
            case SDL_SCANCODE_Z: return Z;
            case SDL_SCANCODE_X: return X;
            case SDL_SCANCODE_C: return C;
            case SDL_SCANCODE_V: return V;
            case SDL_SCANCODE_B: return B;
            case SDL_SCANCODE_N: return N;
            case SDL_SCANCODE_M: return M;
            case SDL_SCANCODE_1: return One;
            case SDL_SCANCODE_2: return Two;
            case SDL_SCANCODE_3: return Three;
            case SDL_SCANCODE_4: return Four;
            case SDL_SCANCODE_5: return Five;
            case SDL_SCANCODE_6: return Six;
            case SDL_SCANCODE_7: return Seven;
            case SDL_SCANCODE_8: return Eight;
            case SDL_SCANCODE_9: return Nine;
            case SDL_SCANCODE_0: return Zero;
            case SDL_SCANCODE_ESCAPE: return Escape;
            case SDL_SCANCODE_SPACE: return Space;
            case SDL_SCANCODE_RETURN: return Enter;
            case SDL_SCANCODE_TAB: return Tab;
            case SDL_SCANCODE_BACKSPACE: return Backspace;
            case SDL_SCANCODE_LSHIFT: return LeftShift;
            case SDL_SCANCODE_RSHIFT: return RightShift;
            case SDL_SCANCODE_LCTRL: return LeftControl;
            case SDL_SCANCODE_RCTRL: return RightControl;
            case SDL_SCANCODE_LGUI: return LeftSuper;
            case SDL_SCANCODE_RGUI: return RightSuper;
            case SDL_SCANCODE_LALT: return LeftAlt;
            case SDL_SCANCODE_RALT: return RightAlt;
            case SDL_SCANCODE_UP: return Up;
            case SDL_SCANCODE_DOWN: return Down;
            case SDL_SCANCODE_LEFT: return Left;
            case SDL_SCANCODE_RIGHT: return Right;
            case SDL_SCANCODE_INSERT: return Insert;
            case SDL_SCANCODE_DELETE: return Delete;
            case SDL_SCANCODE_HOME: return Home;
            case SDL_SCANCODE_END: return End;
            case SDL_SCANCODE_PAGEUP: return PageUp;
            case SDL_SCANCODE_PAGEDOWN: return PageDown;
            case SDL_SCANCODE_F1: return F1;
            case SDL_SCANCODE_F2: return F2;
            case SDL_SCANCODE_F3: return F3;
            case SDL_SCANCODE_F4: return F4;
            case SDL_SCANCODE_F5: return F5;
            case SDL_SCANCODE_F6: return F6;
            case SDL_SCANCODE_F7: return F7;
            case SDL_SCANCODE_F8: return F8;
            case SDL_SCANCODE_F9: return F9;
            case SDL_SCANCODE_F10: return F10;
            case SDL_SCANCODE_F11: return F11;
            case SDL_SCANCODE_F12: return F12;
            case SDL_SCANCODE_SEMICOLON: return Semicolon;
            case SDL_SCANCODE_COMMA: return Comma;
            case SDL_SCANCODE_PERIOD: return Period;
            case SDL_SCANCODE_SLASH: return Slash;
            case SDL_SCANCODE_BACKSLASH: return Backslash;
            case SDL_SCANCODE_APOSTROPHE: return Apostrophe;
            case SDL_SCANCODE_LEFTBRACKET: return LeftBracket;
            case SDL_SCANCODE_RIGHTBRACKET: return RightBracket;
            case SDL_SCANCODE_MINUS: return Minus;
            case SDL_SCANCODE_EQUALS: return Equal;
            case SDL_SCANCODE_GRAVE: return Grave;
            default: return std::nullopt;
        }
    }

    Window::Window(const WindowProperties& properties)
        : m_impl(std::make_unique<Impl>()), m_properties(properties) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string{"Failed to initialize SDL: "} + SDL_GetError());
        }

        m_impl->Handle = SDL_CreateWindow(
            m_properties.Title.c_str(),
            static_cast<int>(m_properties.Width), static_cast<int>(m_properties.Height),
            SDL_WINDOW_VULKAN | (m_properties.Resizable ? SDL_WINDOW_RESIZABLE : 0)
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

    void Window::PollEvents(Input& input) {
        input.beginFrame();

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
                case SDL_EVENT_KEY_DOWN: {
                    const auto key = ToDeoxyKey(event.key.scancode);
                    if (key) input.handleKeyPress(*key);
                    break;
                };
                case SDL_EVENT_KEY_UP: {
                    const auto key = ToDeoxyKey(event.key.scancode);
                    if (key) input.handleKeyRelease(*key);
                    break;
                };
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    input.handleFocusLost();
                    break;
                default: break;
            }
        }
    }

    bool Window::ShouldClose() const { return m_impl->ShouldClose; }

    const WindowProperties& Window::GetProperties() const { return m_properties; }
}
