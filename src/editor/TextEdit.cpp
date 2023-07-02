#include "TextEdit.hpp"
#include "util/Util.hpp"

#include <QKeyEvent>

#include <editor/Editor.hpp>

namespace pico {

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    Editor::getInstance()->forwardEventFilter(this);
}

void
TextEdit::keyPressEvent(QKeyEvent *event)
{
    if (Editor::getInstance()->mode() != util::Mode::Insert)
        return;
    /* Prevent hightlighting with SHFT+ARROW */
    event->setModifiers(Qt::KeyboardModifier::NoModifier);
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }
    this->QTextEdit::keyPressEvent(event);
}

} // namespace pico
