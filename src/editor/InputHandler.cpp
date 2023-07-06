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

/* return true if key was mapped to binding */
bool
InputHandler::handleKeyPress(key64_t key)
{
    // Handle modifiers (modifiers are very large ints)
    // we don't capture mod keys
    if (key & util::QT_CUSTOM_HEX) {
        switch (key) {
        case Qt::Key_Shift:
            m_modifiers.shift++;
            return true;
        case Qt::Key_Control:
            m_modifiers.control++;
            return true;
        case Qt::Key_Alt:
            m_modifiers.alt++;
            return true;
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
        return true;
    } else {
        resetMapIndex();
        return false;
    }
}

bool
InputHandler::handleKeyRelease(key64_t key)
{
    if (key & util::QT_CUSTOM_HEX) {
        /* Alt behaves weirdly */
        switch (key) {
        case Qt::Key_Shift:
            m_modifiers.shift--;
            m_modifiers.alt = 0;
            return true;
        case Qt::Key_Control:
            m_modifiers.control--;
            m_modifiers.alt = 0;
            return true;
        case Qt::Key_Alt:
            m_modifiers.alt = 0;
            return true;
        }
    }
    return false;
}

void
InputHandler::addBinding(QList<QKeyCombination> keys, util::Mode mode, const callback_t &fn)
{
    if (keys.isEmpty())
        return;
    Q_ASSERT(m_keyMapIndex = &m_keyMap);

    key64_t key;
    QList<QKeyCombination>::iterator it_key;
    /* setup the binding tree */
    for (it_key = keys.begin(); it_key < keys.end() - 1; it_key++) {
        key = GEN_KEY64(it_key->toCombined(), mode);
        auto res = m_keyMapIndex->emplace(std::move(key), new keymap_t).first;
        value_t &value = res->second;
        if (value.callable)
            goto err;
        m_keyMapIndex = value.next;
    }
    /* put the callback on leaf node */
    key = GEN_KEY64(it_key->toCombined(), mode);
    if (m_keyMapIndex->find(key) != m_keyMapIndex->end())
        goto err;
    m_keyMapIndex->emplace(std::move(key), callback_t{ std::move(fn) });

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
    using namespace Qt;
    using namespace util;
    auto key = static_cast<QKeyEvent *>(event)->key();

    if (event->type() == QEvent::KeyRelease && key & QT_CUSTOM_HEX) {
        switch (key) {
        case Key_Shift:
        case Key_Control:
        case Key_Alt:
            return handleKeyRelease(key);
        }
    } else if (event->type() == QEvent::KeyPress) {
        if (key & QT_CUSTOM_HEX) {
            switch (key) {
            case Key_Shift:
            case Key_Control:
            case Key_Alt:
                return handleKeyPress(key);
            case Key_Escape:
                setMode(Mode::Normal);
            }
        } else if (m_mode != Mode::Insert || m_modifiers.control || m_modifiers.alt) {
            return handleKeyPress(key);
        }
    } else {
        return QObject::eventFilter(obj, event);
    }
    return false;
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
