#pragma once

namespace pico {

constexpr unsigned MOD_KEY = 0x01000000;

enum Mode {
    Normal = 1 << 0,
    Insert = 1 << 1,
    Visual = 1 << 2,
    VisualLine = 1 << 3,
    VisualBlock = 1 << 4,
    Command = 1 << 5,
    Replace = 1 << 6,
    Binary = 1 << 7,
    Terminal = 1 << 8,
};

} // namespace pico
