#pragma once

#include "editor/KeyListener.hpp"
#include <QTextEdit>

namespace pico {

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit TextEdit(QWidget *parent = nullptr);

protected:
    void
    keyPressEvent(QKeyEvent *event) override;

private:
    KeyListener m_keyListener;
};

} // namespace pico
