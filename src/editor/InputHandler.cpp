#include "InputHandler.hpp"

#include <QKeyEvent>

#include "util/Util.hpp"

#define GEN_KEY64(X, Y) (static_cast<qint64>(X) | (static_cast<qint64>(Y) << 32))

namespace pico {

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
    if (key & util::QT_CUSTOM_HEX) {
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

    key |= Qt::SHIFT * static_cast<bool>(m_modifiers.shift);
    key |= Qt::CTRL * static_cast<bool>(m_modifiers.control);
    key |= Qt::ALT * static_cast<bool>(m_modifiers.alt);

    auto it = m_keyMapIndex->find(GEN_KEY64(key, m_mode));

    if (it != m_keyMapIndex->end()) {
        value_t &val = it->second;
        if (val.callable) {
            val.callback();
            resetMapIndex();
        } else {
            /* traverse the keymap-tree */
            m_keyMapIndex = val.next;
        }
    } else {
        resetMapIndex();
    }
}

void
InputHandler::handleKeyRelease(key64_t key)
{
    if (key & util::QT_CUSTOM_HEX) {
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

    key64_t key;
    QList<QKeyCombination>::iterator it_key;
    for (it_key = keys.begin(); it_key < keys.end() - 1; it_key++) {
        key = GEN_KEY64(it_key->toCombined(), mode);
        auto ret = m_keyMapIndex->emplace(key, new keymap_t).first;
        if (ret->second.callable)
            goto err;
        m_keyMapIndex = ret->second.next;
    }
    key = GEN_KEY64(it_key->toCombined(), mode);
    if (m_keyMapIndex->find(key) != m_keyMapIndex->end())
        goto err;
    m_keyMapIndex->emplace(key, callback_t{ std::move(fn) });

    resetMapIndex();
    return;
err:
    qWarning() << "Binding:" << keys << "failed due to confilcting key combinations";
    resetMapIndex();
}

/* Passively monitor */
[[nodiscard]] bool
InputHandler::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        int key = static_cast<QKeyEvent *>(event)->key();

        if (key & util::QT_CUSTOM_HEX) {
            switch (key) {
            case Qt::Key_Shift:
            case Qt::Key_Control:
            case Qt::Key_Alt:
                handleKeyRelease(static_cast<Qt::Key>(key));
            }
        }
        return false;
    } else if (event->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent *>(event)->key();
        if (m_mode != util::Mode::Insert || key == Qt::Key_Control || key == Qt::Key_Alt ||
            key == Qt::Key_Shift || m_modifiers.control || m_modifiers.alt) {
            auto prevMode = m_mode;
            handleKeyPress(static_cast<Qt::Key>(key));
            if (m_keyMapIndex == &m_keyMap && prevMode == m_mode)
                return false;
            else
                return true;
        } else if (key == Qt::Key_Escape) {
            setMode(util::Mode::Normal);
            return true;
        } else {
            return false;
        }
        return false;
    } else {
        return QObject::eventFilter(obj, event);
    }
}

void
InputHandler::setMode(util::Mode mode)
{
    m_mode = mode;
    resetMapIndex();
}

inline void
InputHandler::resetMapIndex(void)
{
    m_keyMapIndex = &m_keyMap;
}

} // namespace pico
