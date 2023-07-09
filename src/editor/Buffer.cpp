#include "Buffer.hpp"

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QWidget(parent),
      PicoObject(this),
      m_layout(new QGridLayout(this))
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    auto *textEdit = new TextEdit(this);
    m_layout->addWidget(textEdit);
}

} // namespace pico
