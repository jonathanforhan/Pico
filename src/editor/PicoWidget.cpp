#include "PicoWidget.hpp"
#include "editor/Editor.hpp"

#include <QApplication>

namespace pico {

PicoWidget::PicoWidget(QWidget *child)
    : m_keyListener({}),
      m_child(child)
{
    Editor::getInstance()->forwardKeyFilter(child);
    child->setFont(QApplication::font());
}

bool
PicoWidget::handleKeyPress(qint64 key)
{
    return m_keyListener.handleKeyPress(key);
}

bool
PicoWidget::addBinding(QList<QKeyCombination> keyCombo, Mode mode, std::function<void()> callback)
{
    return m_keyListener.addBinding(keyCombo, mode, callback);
}

} // namespace pico
