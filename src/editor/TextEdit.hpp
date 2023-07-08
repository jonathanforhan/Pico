#pragma once

#include "editor/KeyListener.hpp"
#include "editor/PicoObject.hpp"
#include <QTextEdit>

namespace pico {

class TextEdit : public QTextEdit, public PicoObject
{
    Q_OBJECT

public:
    explicit TextEdit(QWidget *parent = nullptr);

protected:
    void
    keyPressEvent(QKeyEvent *event) override;
};

} // namespace pico
