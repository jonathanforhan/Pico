#include "PicoObject.hpp"
#include "editor/Editor.hpp"

namespace pico {

PicoObject::PicoObject(QObject *obj)
    : m_keyListener({}),
      m_child(obj)
{
    Editor::getInstance()->forwardKeyFilter(obj);
}

bool
PicoObject::handleKeyPress(qint64 key)
{
    return m_keyListener.handleKeyPress(key);
}

bool
PicoObject::addBinding(QList<QKeyCombination> keyCombo, Mode mode, std::function<void()> callback)
{
    return m_keyListener.addBinding(keyCombo, mode, callback);
}

} // namespace pico
