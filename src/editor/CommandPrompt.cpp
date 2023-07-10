#include "CommandPrompt.hpp"

#include <QProcess>
#include <iostream>

#include <editor/Editor.hpp>

namespace pico {

CommandPrompt::CommandPrompt(QWidget *parent)
    : QPlainTextEdit(parent),
      PicoWidget(this),
      m_previousCommands({}),
      m_index(0)
{
    setMaximumHeight(32);
    appendPlainText(":");
    m_previousCommands.push_front(":");
}

void
CommandPrompt::keyPressEvent(QKeyEvent *event)
{
    using namespace Qt;
    auto editor = Editor::getInstance();

    if (editor->mode() != Mode::Command)
        return;

    m_previousCommands[m_index] = toPlainText();
    auto key = event->key();

    if (key == Key_Backspace && toPlainText() == ":") {
        return;
    } else if (key == Key_Enter || key == Key_Return) {
        auto command = toPlainText();
        if (command.count() >= 2 && command[1] == '!') {
            executeCommand(command.sliced(2));
        } else {
            qWarning() << "internal commands not yet supported";
        }
        setPlainText("");
        appendPlainText(":");
        if (m_index == 0)
            m_previousCommands.push_front(":");
        m_index = 0;
        editor->setMode(Mode::Normal);

    } else if (key == Key_Up) {
        if (m_index + 1 == m_previousCommands.count())
            return;
        setPlainText("");
        appendPlainText(m_previousCommands.at(++m_index));
    } else if (key == Key_Down) {
        if (m_index == 0)
            return;
        setPlainText("");
        appendPlainText(m_previousCommands.at(--m_index));
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

void
CommandPrompt::executeCommand(const QString &cmd)
{
    // TODO
    QProcess::execute(cmd);
    // QProcess proc;
    // proc.start(cmd);
    // if (!proc.waitForStarted())
    //     return;
    //
    // if (!proc.waitForFinished())
    //     return;
    //
    // auto result = proc.readAll();
    // std::cout << result.toStdString() << "\n";
}

} // namespace pico
