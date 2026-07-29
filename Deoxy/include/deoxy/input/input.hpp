#pragma once

#include <deoxy/input/key.hpp>

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
        private:
            friend class deoxy::platform::Window;

            static constexpr size_t KEY_COUNT = static_cast<size_t>(Key::Count);
            static constexpr size_t toIndex(Key key) { return static_cast<size_t>(key); }

            void beginFrame();
            void handleKeyPress(Key key);
            void handleKeyRelease(Key key);
            void handleFocusLost();

            std::bitset<KEY_COUNT> m_pressed;
            std::bitset<KEY_COUNT> m_pressedThisFrame;
            std::bitset<KEY_COUNT> m_releasedThisFrame;
    };
}
