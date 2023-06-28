#pragma once

#include <Qt>

namespace pico {

enum ModKey {
    None = 0x0000,
    Shift = 0x0100,
    Control = 0x0200,
    Alt = 0x0400,
};

enum Mode {
    Normal = 0x0800,
    Insert = 0x1000,
    Visual = 0x2000,
    VisualBlock = 0x4000,
    Command = 0x8000,
};

enum Key {
    /* Ascii */
    /* the comments to the side are the Qt equivalents and do not represent
     * the pico API bindings nor do they represent the ascii bindings */
    Space = Qt::Key_Space,
    Exclam = Qt::Key_1 | ModKey::Shift,            // Exclam = Qt::Key_Exclam,
    QuoteDbl = Qt::Key_Apostrophe | ModKey::Shift, // QuoteDbl = Qt::Key_QuoteDbl,
    NumberSign = Qt::Key_3 | ModKey::Shift,        // NumberSign = Qt::Key_NumberSign,
    Dollar = Qt::Key_4 | ModKey::Shift,            // Dollar = Qt::Key_Dollar,
    Percent = Qt::Key_5 | ModKey::Shift,           // Percent = Qt::Key_Percent,
    Ampersand = Qt::Key_7 | ModKey::Shift,         // Ampersand = Qt::Key_Ampersand,
    Apostrophe = Qt::Key_Apostrophe,
    ParenLeft = Qt::Key_9 | ModKey::Shift,  // ParenLeft = Qt::Key_ParenLeft,
    ParenRight = Qt::Key_0 | ModKey::Shift, // ParenRight = Qt::Key_ParenRight,
    Asterisk = Qt::Key_8 | ModKey::Shift,   // Asterisk = Qt::Key_Asterisk,
    Plus = Qt::Key_Equal | ModKey::Shift,   // Plus = Qt::Key_Plus,
    Comma = Qt::Key_Comma,
    Minus = Qt::Key_Minus,
    Period = Qt::Key_Period,
    Slash = Qt::Key_Slash,
    _0 = Qt::Key_0,
    _1 = Qt::Key_1,
    _2 = Qt::Key_2,
    _3 = Qt::Key_3,
    _4 = Qt::Key_4,
    _5 = Qt::Key_5,
    _6 = Qt::Key_6,
    _7 = Qt::Key_7,
    _8 = Qt::Key_8,
    _9 = Qt::Key_9,
    Colon = Qt::Key_Semicolon | ModKey::Shift, // Colon = Qt::Key_Colon,
    Semicolon = Qt::Key_Semicolon,
    Less = Qt::Key_Comma | ModKey::Shift, // Less = Qt::Key_Less,
    Equal = Qt::Key_Equal,
    Greater = Qt::Key_Period | ModKey::Shift, // Greater = Qt::Key_Greater,
    Question = Qt::Key_Slash | ModKey::Shift, // Question = Qt::Key_Question,
    A = Qt::Key_A,
    B = Qt::Key_B,
    C = Qt::Key_C,
    D = Qt::Key_D,
    E = Qt::Key_E,
    F = Qt::Key_F,
    G = Qt::Key_G,
    H = Qt::Key_H,
    I = Qt::Key_I,
    J = Qt::Key_J,
    K = Qt::Key_K,
    L = Qt::Key_L,
    M = Qt::Key_M,
    N = Qt::Key_N,
    O = Qt::Key_O,
    P = Qt::Key_P,
    Q = Qt::Key_Q,
    R = Qt::Key_R,
    S = Qt::Key_S,
    T = Qt::Key_T,
    U = Qt::Key_U,
    V = Qt::Key_V,
    W = Qt::Key_W,
    X = Qt::Key_X,
    Y = Qt::Key_Y,
    Z = Qt::Key_Z,
};

} // namespace pico
