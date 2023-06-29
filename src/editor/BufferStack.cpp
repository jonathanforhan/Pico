#include "BufferStack.hpp"

#include "editor/Editor.hpp"

namespace pico {

BufferStack::BufferStack(Editor *parent)
    : QStackedLayout(dynamic_cast<QWidget *>(parent)),
      m_inputHandler(parent->getInputHandler())
{}

void
BufferStack::addWidget(Buffer *buffer)
{
    buffer->installEventFilter(buffer);
    buffer->setFont(QFont("JetBrains Mono NF", 12));
    this->QStackedLayout::addWidget(buffer);
}

} // namespace pico
