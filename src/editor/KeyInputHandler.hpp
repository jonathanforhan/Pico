#pragma once

#include <QKeyEvent>
#include <QObject>

#include "editor/KeyEventFilter.hpp"

namespace pico {

namespace KeyBind {
enum Mod {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,
};
}

class KeyInputHandler : public QObject
{
    Q_OBJECT

public:
    explicit KeyInputHandler(QObject *parent = nullptr);

    const KeyEventFilter *
    getKeyEventFilter();

    bool
    addBinding(void (*function)(), Qt::Key key, int mod = KeyBind::Mod::None);

public slots:
    void
    handleKeyPress(Qt::Key key);

    void
    handleKeyRelease(Qt::Key key);

private:
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modifiers;

    KeyEventFilter *m_keyEventFilter;
    std::array<void (*)(), 1028> m_keyBindings;
    static const unsigned m_keyOffsets = 128;
};

} // namespace pico
