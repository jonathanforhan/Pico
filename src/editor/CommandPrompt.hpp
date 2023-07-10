#pragma once

#include <QPlainTextEdit>

#include "editor/PicoWidget.hpp"

namespace pico {

class CommandPrompt : public QPlainTextEdit, public PicoWidget
{
    Q_OBJECT

public:
    explicit CommandPrompt(QWidget *parent = nullptr);

    void
    keyPressEvent(QKeyEvent *event);

    void
    executeCommand(const QString &cmd);

private:
    QList<QString> m_previousCommands;
    int m_index;
};

} // namespace pico
