#include <deoxy/input/input.hpp>

namespace deoxy {
    void Input::beginFrame() {
        m_pressedThisFrame.reset();
        m_releasedThisFrame.reset();
        m_mousePressedThisFrame.reset();
        m_mouseReleasedThisFrame.reset();

        m_mouseDelta = math::Vec2{0.0f};
        m_mouseWheelDelta = math::Vec2{0.0f};
    }

    void Input::handleKeyPress(Key key) {
        const std::size_t index = toIndex(key);
        if (!m_pressed.test(index)) m_pressedThisFrame.set(index);

        m_pressed.set(index);
    }

    void Input::handleKeyRelease(Key key) {
        const std::size_t index = toIndex(key);
        if (m_pressed.test(index)) m_releasedThisFrame.set(index);

        m_pressed.reset(index);
    }

    void Input::handleMousePress(Mouse btn) {
        const std::size_t index = toIndex(btn);
        if (!m_mousePressed.test(index)) m_mousePressedThisFrame.set(index);

        m_mousePressed.set(index);
    }

    void Input::handleMouseRelease(Mouse btn) {
        const std::size_t index = toIndex(btn);
        if (m_mousePressed.test(index)) m_mouseReleasedThisFrame.set(index);

        m_mousePressed.reset(index);
    }

    void Input::handleMouseMotion(float x, float y, float deltaX, float deltaY) {
        m_mousePosition = math::Vec2{x, y};
        m_mouseDelta += math::Vec2{deltaX, deltaY};
    }

    void Input::handleMouseWheel(float x, float y) {
        m_mouseWheelDelta += math::Vec2{x, y};
    }

    void Input::handleFocusLost() {
        m_releasedThisFrame |= m_pressed;
        m_pressed.reset();

        m_mouseReleasedThisFrame |= m_mousePressed;
        m_mousePressed.reset();

        m_mouseDelta = math::Vec2{0.0f};
        m_mouseWheelDelta = math::Vec2{0.0f};
    }
}
