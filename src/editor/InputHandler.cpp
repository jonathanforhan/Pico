#include "InputHandler.hpp"

#include <QKeyEvent>

#include "util/Util.hpp"

#define QT_CUSTOM_HEX 0x01000000 /* Indicates a QT API enum (non unicode confilcting) */

namespace pico {
using namespace binding;
using std::make_shared;

/* Public */

InputHandler::InputHandler(QObject *parent)
    : QObject(parent),
      m_modifiers({}),
      m_mode(Mode::Normal),
      m_keyMap(keymap_t{}),
      m_keyMapIndex(&m_keyMap)
{}

void
InputHandler::handleKeyPress(key_t key)
{
    // Handle modifiers (modifiers are very large ints)
    if (key & QT_CUSTOM_HEX) {
        switch (key) {
        case Qt::Key_Shift:
            m_modifiers.shift++;
            return;
        case Qt::Key_Control:
            m_modifiers.control++;
            return;
        case Qt::Key_Alt:
            m_modifiers.alt++;
            return;
        default:
            break;
        }
    }

    int mod = ModKey::None;
    mod |= ModKey::Shift * !!m_modifiers.shift;
    mod |= ModKey::Control * !!m_modifiers.control;
    mod |= ModKey::Alt * !!m_modifiers.alt;

    /* if the macro is completed, call it, else if it's a chord
     * conituation, traverse it, else reset keyMapIndex */
    auto iter = m_keyMapIndex->find(key | mod | m_mode);

    if (iter != m_keyMapIndex->end()) {
        entry_t entry = iter->second;

        if (entry.callable) {
            auto func = *static_cast<callback_t *>(entry.link.get());
            func();
            resetMapIndex();
        } else {
            m_keyMapIndex = static_cast<keymap_t *>(entry.link.get());
        }
    } else {
        resetMapIndex();
    }
}

void
InputHandler::handleKeyRelease(key_t key)
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
InputHandler::addBinding(QList<key_t> keys, Mode mode, const callback_t &fn)
{
    if (keys.isEmpty())
        return;
    Q_ASSERT(m_keyMapIndex = &m_keyMap);

    int i;
    for (i = 0; i < keys.size() - 1; i++) {
        key_t key = keys[i] | mode;
        entry_t map_entry = { make_shared<keymap_t>(), false };
        map_entry = m_keyMapIndex->emplace(key, std::move(map_entry)).first->second;
        m_keyMapIndex = static_cast<keymap_t *>(map_entry.link.get());
    }
    key_t key = keys[i] | mode;
    entry_t func_entry = { make_shared<callback_t>(std::move(fn)), true };
    m_keyMapIndex->emplace(key, std::move(func_entry));

    resetMapIndex();
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
                handleKeyRelease(key);
                return true;
            default:
                return false;
            }
        }
        return false;
    } else if (event->type() == QEvent::KeyPress && m_mode != Mode::Insert) {
        auto key = static_cast<QKeyEvent *>(event)->key();
        handleKeyPress(key);
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void
InputHandler::setMode(Mode mode)
{
    m_mode = mode;
}

Mode
InputHandler::getMode(void)
{
    return m_mode;
}

/* Private */

void
InputHandler::resetMapIndex(void)
{
    m_keyMapIndex = &m_keyMap;
}

} // namespace pico
