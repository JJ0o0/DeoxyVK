#include <deoxy/input/input.hpp>

namespace deoxy {
    void Input::beginFrame() {
        m_pressedThisFrame.reset();
        m_releasedThisFrame.reset();
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

    void Input::handleFocusLost() {
        m_releasedThisFrame |= m_pressed;
        m_pressed.reset();
    }
}
