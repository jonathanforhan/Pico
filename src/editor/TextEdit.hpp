#pragma once

#include <QTextEdit>

namespace pico {

class TextEdit : public QTextEdit
{
public:
    explicit TextEdit(QWidget *parent = nullptr);

    void
    keyPressEvent(QKeyEvent *event) override;

private:
};

} // namespace pico
