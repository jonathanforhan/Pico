#include "KeyEventFilter.hpp"

namespace pico {

KeyEventFilter::KeyEventFilter(QObject *parent)
    : QObject(parent)
{}

bool
KeyEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        m_key = static_cast<QKeyEvent *>(event);

        switch (m_key->key()) {
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
            keyRelease((Qt::Key)m_key->key());
            break;
        default:
            return false;
        }
        return true;

    } else if (event->type() == QEvent::KeyPress) {
        m_key = static_cast<QKeyEvent *>(event);

        keyPress((Qt::Key)m_key->key());
        return true;

    } else {
        return QObject::eventFilter(obj, event);
    }
}

} // namespace pico
