#pragma once

#include "editor/KeyListener.hpp"
namespace pico {

class PicoObject
{
protected:
    explicit PicoObject(QObject *obj);

    virtual bool
    handleKeyPress(qint64 key);

    bool
    addBinding(QList<QKeyCombination> keyCombo, Mode mode, std::function<void()> callback);

private:
    KeyListener m_keyListener;
    QObject *m_child;
};

} // namespace pico
