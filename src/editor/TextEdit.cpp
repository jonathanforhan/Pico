#include "TextEdit.hpp"
#include "util/Util.hpp"

#include <QKeyEvent>

#include <editor/Editor.hpp>

namespace pico {

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    Editor::getInstance()->forwardEventFilter(this);
    this->setCursorWidth(9);
}

void
TextEdit::keyPressEvent(QKeyEvent *event)
{
    using namespace Qt;
    using namespace util;
    auto editor = Editor::getInstance();
    auto key = event->key();

    if (editor->mode() == Mode::Normal) {
        switch (key) {
        case Key_H:
            return moveCursor(QTextCursor::MoveOperation::Left);
        case Key_J:
            return moveCursor(QTextCursor::MoveOperation::Down);
        case Key_K:
            return moveCursor(QTextCursor::MoveOperation::Up);
        case Key_L:
            return moveCursor(QTextCursor::MoveOperation::Right);
        case Key_W:
            if (editor->isShiftPressed())
                return moveCursor(QTextCursor::MoveOperation::NextCell);
            else
                return moveCursor(QTextCursor::MoveOperation::NextWord);
        case Key_E:
            return moveCursor(QTextCursor::MoveOperation::WordRight);
        case Key_B:
            if (editor->isShiftPressed())
                return moveCursor(QTextCursor::MoveOperation::PreviousBlock);
            else
                return moveCursor(QTextCursor::MoveOperation::PreviousWord);
        case Key_P:
            return paste();
        case Key_U:
            return undo();
        }
    }

    /* Only insert mode beyond this point */
    if (editor->mode() != Mode::Insert)
        return;

    /* Prevent hightlighting with SHFT+ARROW */
    event->setModifiers(KeyboardModifier::NoModifier);
    if (event->key() == Key_Tab) {
        insertPlainText("    ");
        return;
    }
    this->QTextEdit::keyPressEvent(event);
}

} // namespace pico
