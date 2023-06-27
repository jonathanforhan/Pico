#include "KeyInputHandler.hpp"

namespace pico {

KeyInputHandler::KeyInputHandler(QObject *parent)
    : QObject(parent),
      m_modifiers({}),
      m_keyEventFilter(new KeyEventFilter(this)),
      m_keyBindings({})
{
    connect(m_keyEventFilter, &KeyEventFilter::keyPress, this, &KeyInputHandler::handleKeyPress);
    connect(m_keyEventFilter, &KeyEventFilter::keyRelease, this,
            &KeyInputHandler::handleKeyRelease);
}

const KeyEventFilter *
KeyInputHandler::getKeyEventFilter()
{
    return this->m_keyEventFilter;
}

bool
KeyInputHandler::addBinding(void (*function)(), Qt::Key key, int mod)
{
    unsigned keyValue = static_cast<int>(key);

    keyValue += m_keyOffsets * mod;

    if (keyValue >= m_keyBindings.size())
        return false;
    if (m_keyBindings[keyValue] != nullptr)
        return false;

    m_keyBindings[keyValue] = function;

    return true;
}

void
KeyInputHandler::handleKeyPress(Qt::Key key)
{
    // Handle modifiers (modifiers are very large ints)
    if (key >= m_keyBindings.size()) {
        switch (key) {
        case Qt::Key_Shift:
            m_modifiers.shift++;
            break;
        case Qt::Key_Control:
            m_modifiers.control++;
            break;
        case Qt::Key_Alt:
            m_modifiers.alt++;
            break;
        default:
            break;
        }
        return;
    }

    unsigned keyValue = static_cast<int>(key);

    // mutate keyValue with modifiers that are held, this !!
    // ensure that even if both shift keys, etc are held it will
    // only multiply by one, or if false its keyValue += 0
    keyValue += m_keyOffsets * !!m_modifiers.shift * KeyBind::Mod::Shift;
    keyValue += m_keyOffsets * !!m_modifiers.control * KeyBind::Mod::Control;
    keyValue += m_keyOffsets * !!m_modifiers.alt * KeyBind::Mod::Alt;

    if (m_keyBindings[keyValue] != nullptr)
        m_keyBindings[keyValue]();
}

void
KeyInputHandler::handleKeyRelease(Qt::Key key)
{
    // Alt behaves weirdly
    switch (key) {
    case Qt::Key_Shift:
        m_modifiers.shift--;
        m_modifiers.alt = 0;
        break;
    case Qt::Key_Control:
        m_modifiers.control--;
        m_modifiers.alt = 0;
        break;
    case Qt::Key_Alt:
        m_modifiers.alt = 0;
        break;
    default:
        break;
    }
}

} // namespace pico
