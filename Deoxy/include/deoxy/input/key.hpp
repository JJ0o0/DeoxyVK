#pragma once

#include <cstdint>

namespace deoxy::input {
    enum class Key : uint16_t {
        Q, W, E, R, T, Y, U, I, O, P,
        A, S, D, F, G, H, J, K, L,
        Z, X, C, V, B, N, M,

        One, Two, Three,
        Four, Five, Six,
        Seven, Eight, Nine,
        Zero,

        Escape, Space, Enter, Tab, Backspace,

        LeftShift, RightShift,
        LeftControl, RightControl,
        LeftSuper, RightSuper,
        LeftAlt, RightAlt,

        Up, Down, Left, Right,

        Insert, Delete, Home, End, PageUp, PageDown,

        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

        Semicolon, Comma, Period,
        Slash, Backslash, Apostrophe,
        LeftBracket, RightBracket,
        Minus, Equal, Grave,

        Count
    };
}
