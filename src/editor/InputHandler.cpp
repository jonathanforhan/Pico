#include "InputHandler.hpp"

#include <QKeyEvent>

#define QT_CUSTOM_HEX 0x01000000 /* Indicates a QT API enum (non unicode confilcting) */
#define CALL(F) F()              /* provide cleaner call to callback */

namespace pico {
using namespace binding;

InputHandler::InputHandler(QObject *parent)
    : QObject(parent),
      m_keyMap(keymap_t{}),
      m_keyMapIndex(&m_keyMap)
{}

void
InputHandler::handleKeyPress(Qt::Key key)
{
    // Handle modifiers (modifiers are very large ints)
    if (key & QT_CUSTOM_HEX) {
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
        case Qt::Key_Left:
            handleKeyPress(Qt::Key_H); // FIXME
            break;
        case Qt::Key_Down:
            handleKeyPress(Qt::Key_J); // FIXME
            break;
        case Qt::Key_Up:
            handleKeyPress(Qt::Key_K); // FIXME
            break;
        case Qt::Key_Right:
            handleKeyPress(Qt::Key_L); // FIXME
            break;
        default:
            m_keyMapIndex = &m_keyMap;
        }
        return;
    }

    int mod = Mod::None;
    mod |= Mod::Shift * !!m_modifiers.shift;
    mod |= Mod::Control * !!m_modifiers.control;
    mod |= Mod::Alt * !!m_modifiers.alt;

    /* if the macro is completed, call it, else if it's a chord
     * conituation, traverse it, else reset keyMapIndex */
    auto entry = m_keyMapIndex->find(key | mod);
    if (entry != m_keyMapIndex->end()) {
        if (entry->second.second == true) {
            CALL((*static_cast<callback_t *>(entry->second.first.get())));
            m_keyMapIndex = &m_keyMap;
        } else {
            m_keyMapIndex = static_cast<keymap_t *>(entry->second.first.get());
        }
    } else {
        m_keyMapIndex = &m_keyMap;
    }
}

void
InputHandler::handleKeyRelease(Qt::Key key)
{
    if (key & QT_CUSTOM_HEX) {
        /* Alt behaves weirdly */
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
}

void
InputHandler::addBinding(QList<key_t> keys, const callback_t &fn)
{
    if (keys.isEmpty())
        return;

    Q_ASSERT(m_keyMapIndex = &m_keyMap);

    int i;
    for (i = 0; i < keys.size() - 1; i++) {
        auto val = m_keyMapIndex->insert({ keys[i], { std::make_shared<keymap_t>(), false } });
        m_keyMapIndex = static_cast<keymap_t *>(val.first->second.first.get());
    }
    m_keyMapIndex->insert({ keys[i], { std::make_shared<callback_t>(std::move(fn)), true } });

    m_keyMapIndex = &m_keyMap;
}

[[nodiscard]] bool
InputHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        auto key = static_cast<QKeyEvent *>(event)->key();

        if (key & QT_CUSTOM_HEX) {
            switch (key) {
            case Qt::Key_Shift:
            case Qt::Key_Control:
            case Qt::Key_Alt:
                handleKeyRelease(static_cast<Qt::Key>(key));
                return true;
            default:
                return false;
            }
        }
        return false;
    } else if (event->type() == QEvent::KeyPress) {
        auto key = static_cast<QKeyEvent *>(event)->key();
        handleKeyPress(static_cast<Qt::Key>(key));
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

} // namespace pico
