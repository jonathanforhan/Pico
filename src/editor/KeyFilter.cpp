#include "KeyFilter.hpp"
#include "util/Util.hpp"

#include <QEvent>
#include <QKeyEvent>

#include <editor/Editor.hpp>

#define GEN_KEY64(X, Y) (static_cast<qint64>(X) | (static_cast<qint64>(Y) << 32))

namespace pico {

KeyFilter::KeyFilter(QObject *parent)
    : KeyListener(parent),
      m_editor(Editor::getInstance())
{}

bool
KeyFilter::handleKeyRelease(key64_t key)
{
    Editor *editor = static_cast<Editor *>(m_editor);

    switch (key) {
    case Qt::Key_Shift:
        editor->m_modifiers.shift--;
        editor->m_modifiers.alt = 0;
        return true;
    case Qt::Key_Control:
        editor->m_modifiers.control--;
        editor->m_modifiers.alt = 0;
        return true;
    case Qt::Key_Alt:
        editor->m_modifiers.alt = 0;
        return true;
    }
    return false;
}

bool
KeyFilter::handleKeyPress(key64_t key)
{
    Editor *editor = static_cast<Editor *>(m_editor);

    switch (key) {
    case Qt::Key_Shift:
        editor->m_modifiers.shift++;
        return true;
    case Qt::Key_Control:
        editor->m_modifiers.control++;
        return true;
    case Qt::Key_Alt:
        editor->m_modifiers.alt++;
        return true;
    }

    key |= Qt::SHIFT * editor->shiftState();
    key |= Qt::CTRL * editor->controlState();
    key |= Qt::ALT * editor->altState();

    auto it = m_keyMapIndex->find(GEN_KEY64(key, editor->mode()));

    if (it != m_keyMapIndex->end()) {
        value_t &val = it->second;
        if (val.callable) {
            val.callback();
            resetMapIndex();
            return true;
        } else {
            /* traverse the keymap-tree */
            m_keyMapIndex = val.next;
            return false; /* return false to not block keys of remaps */
        }
    } else {
        resetMapIndex();
        return false;
    }
}

bool
KeyFilter::eventFilter(QObject *obj, QEvent *event)
{
    Editor *editor = static_cast<Editor *>(m_editor);

    if (event->type() == QEvent::KeyRelease) {
        Qt::Key key = static_cast<Qt::Key>(static_cast<QKeyEvent *>(event)->key());
        return handleKeyRelease(key);

    } else if (event->type() == QEvent::KeyPress) {
        Qt::Key key = static_cast<Qt::Key>(static_cast<QKeyEvent *>(event)->key());
        /* Escape is hardcoded into event handler */
        if (key == Qt::Key_Escape) {
            editor->setMode(Mode::Normal);
            return false;
        } else {
            return handleKeyPress(key);
        }
    } else {
        return QObject::eventFilter(obj, event);
    }
}

} // namespace pico
