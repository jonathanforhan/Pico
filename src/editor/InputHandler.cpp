#include "InputHandler.hpp"

#include <QKeyEvent>

#include "util/Util.hpp"

#define QT_CUSTOM_HEX 0x01000000 /* Indicates a QT API enum (non unicode confilcting) */
#define GEN_KEY64(X, Y) (static_cast<qint64>(X) | (static_cast<qint64>(Y) << 32))

namespace pico {
using namespace binding;

/* Public */

InputHandler::InputHandler(QObject *parent)
    : QObject(parent),
      m_modifiers({}),
      m_mode(util::Mode::Normal),
      m_keyMap(keymap_t{}),
      m_keyMapIndex(&m_keyMap)
{}

void
InputHandler::handleKeyPress(key64_t key)
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

    key |= Qt::SHIFT * !!m_modifiers.shift;
    key |= Qt::CTRL * !!m_modifiers.control;
    key |= Qt::ALT * !!m_modifiers.alt;

    auto it = m_keyMapIndex->find(GEN_KEY64(key, m_mode));

    if (it != m_keyMapIndex->end()) {
        value_t &val = it->second;
        if (val.callable) {
            val.callback();
            resetMapIndex();
        } else {
            m_keyMapIndex = val.next;
        }
    } else {
        resetMapIndex();
    }
}

void
InputHandler::handleKeyRelease(key64_t key)
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
InputHandler::addBinding(QList<QKeyCombination> keys, util::Mode mode, const callback_t &fn)
{
    if (keys.isEmpty())
        return;
    Q_ASSERT(m_keyMapIndex = &m_keyMap);

    binding::key64_t key;
    QList<QKeyCombination>::iterator it_key;
    for (it_key = keys.begin(); it_key < keys.end() - 1; it_key++) {
        key = GEN_KEY64(it_key->toCombined(), mode);
        auto ret = m_keyMapIndex->emplace(key, new keymap_t).first;
        m_keyMapIndex = ret->second.next;
    }
    key = GEN_KEY64(it_key->toCombined(), mode);
    m_keyMapIndex->emplace(key, callback_t{ std::move(fn) });

    resetMapIndex();
}

[[nodiscard]] bool
InputHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        int key = static_cast<QKeyEvent *>(event)->key();

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
    } else if (event->type() == QEvent::KeyPress && m_mode != util::Mode::Insert) {
        int key = static_cast<QKeyEvent *>(event)->key();
        handleKeyPress(static_cast<Qt::Key>(key));
        return true;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void
InputHandler::setMode(util::Mode mode)
{
    m_mode = mode;
}

util::Mode
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
