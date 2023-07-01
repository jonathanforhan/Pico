#pragma once

namespace pico {
namespace util {

enum Mode {
    Normal = 0,
    Insert = 1,
    Visual = 2,
    VisualBlock = 3,
    Command = 4,
};

constexpr int NumberOfModes = 5;

} // namespace util
} // namespace pico
