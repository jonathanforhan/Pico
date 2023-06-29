#include "TextBuffer.hpp"

#include <QKeyEvent>

#include "editor/Editor.hpp"

namespace pico {

TextBuffer::TextBuffer(Editor *parent)
    : Buffer(parent),
      m_editor(parent)
{}

[[nodiscard]] bool
TextBuffer::eventFilter(QObject *obj, QEvent *event)
{
    if (m_editor->getMode() == Mode::Insert) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            m_editor->setMode(Mode::Normal);
        } else {
            keyPressEvent(keyEvent);
        }
        return true;
    } else {
        return m_editor->getInputHandler()->eventFilter(obj, event);
    }
}

} // namespace pico
