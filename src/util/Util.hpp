#pragma once

namespace pico {
namespace util {

enum Mode {
    Normal = 1 << 0,
    Insert = 1 << 1,
    Visual = 1 << 2,
    VisualBlock = 1 << 3,
    Command = 1 << 4,
};

/* Indicates a QT API enum (non unicode confilcting) */
constexpr unsigned QT_CUSTOM_HEX = 0x01000000;

} // namespace util
} // namespace pico
