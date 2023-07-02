#include "editor/Buffer.hpp"

#include <QKeyEvent>
#include <qnamespace.h>

#include "editor/Editor.hpp"

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QTextEdit(parent),
      m_children({}),
      m_grid(nullptr)
{
    Editor *editor = Editor::getInstance();
    editor->forwardEventFilter(this);
    setFont(Editor::getInstance()->font());
    insertPlainText("Foo bar");
}

void
Buffer::keyPressEvent(QKeyEvent *event)
{
    /* Prevent hightlighting with SHFT+ARROW */
    event->setModifiers(Qt::KeyboardModifier::NoModifier);
    if (event->key() == Qt::Key_Tab) {
        insertPlainText("    ");
        return;
    }
    this->QTextEdit::keyPressEvent(event);
}

} // namespace pico
