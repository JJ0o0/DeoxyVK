#pragma once

#include <deoxy/input/key.hpp>
#include <deoxy/input/mouse_buttons.hpp>
#include <deoxy/math/vec2.hpp>

#include <bitset>

namespace deoxy::platform {
    class Window;
}

using namespace deoxy::input;
namespace deoxy {
    class Input {
        public:
            bool IsPressed(Key key) const { return m_pressed.test(toIndex(key)); }
            bool IsReleased(Key key) const { return !IsPressed(key); }

            bool WasPressed(Key key) const { return m_pressedThisFrame.test(toIndex(key)); }
            bool WasReleased(Key key) const { return m_releasedThisFrame.test(toIndex(key)); }

            bool IsMousePressed(Mouse btn) const { return m_mousePressed.test(toIndex(btn)); }
            bool IsMouseReleased(Mouse btn) const { return !IsMousePressed(btn); }

            bool WasMousePressed(Mouse btn) const { return m_mousePressedThisFrame.test(toIndex(btn)); }
            bool WasMouseReleased(Mouse btn) const { return m_mouseReleasedThisFrame.test(toIndex(btn)); }

            math::Vec2 GetMousePosition() const { return m_mousePosition; }
            math::Vec2 GetMouseDelta() const { return m_mouseDelta; }
            math::Vec2 GetMouseWheelDelta() const { return m_mouseWheelDelta; }
        private:
            friend class deoxy::platform::Window;

            static constexpr size_t KEY_COUNT = static_cast<size_t>(Key::Count);
            static constexpr size_t MOUSE_BUTTON_COUNT = static_cast<size_t>(Mouse::Count);

            static constexpr size_t toIndex(Key key) { return static_cast<size_t>(key); }
            static constexpr size_t toIndex(Mouse btn) { return static_cast<size_t>(btn); }

            void beginFrame();
            void handleKeyPress(Key key);
            void handleKeyRelease(Key key);
            void handleMousePress(Mouse btn);
            void handleMouseRelease(Mouse btn);
            void handleMouseMotion(float x, float y, float deltaX, float deltaY);
            void handleMouseWheel(float x, float y);
            void handleFocusLost();

            math::Vec2 m_mousePosition{0.0f};
            math::Vec2 m_mouseDelta{0.0f};
            math::Vec2 m_mouseWheelDelta{0.0f};

            std::bitset<MOUSE_BUTTON_COUNT> m_mousePressed;
            std::bitset<MOUSE_BUTTON_COUNT> m_mousePressedThisFrame;
            std::bitset<MOUSE_BUTTON_COUNT> m_mouseReleasedThisFrame;

            std::bitset<KEY_COUNT> m_pressed;
            std::bitset<KEY_COUNT> m_pressedThisFrame;
            std::bitset<KEY_COUNT> m_releasedThisFrame;
    };
}
