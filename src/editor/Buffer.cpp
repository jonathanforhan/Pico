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
}

} // namespace pico
