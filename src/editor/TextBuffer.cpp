#include "TextBuffer.hpp"

#include <QKeyEvent>

#include "editor/Editor.hpp"

namespace pico {

TextBuffer::TextBuffer(QWidget *parent)
    : Buffer(parent)
{}

void
TextBuffer::keyPressEvent(QKeyEvent *event)
{
    auto e = Editor::getInstance();
    if (event->key() == Qt::Key_Escape) {
        modeChange(Mode::Normal);
    } else if (e->getMode() == Mode::Insert) {
        Buffer::keyPressEvent(event);
    }
}

} // namespace pico
