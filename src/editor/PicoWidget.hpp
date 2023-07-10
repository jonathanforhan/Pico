#pragma once

#include "editor/KeyListener.hpp"
namespace pico {

class PicoWidget
{
protected:
    explicit PicoWidget(QWidget *child);

    virtual bool
    handleKeyPress(qint64 key);

public:
    bool
    addBinding(QList<QKeyCombination> keyCombo, Mode mode, std::function<void()> callback);

private:
    KeyListener m_keyListener;
    QObject *m_child;
};

} // namespace pico
