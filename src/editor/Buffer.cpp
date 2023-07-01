#include "editor/Buffer.hpp"

#include <QKeyEvent>

#include "editor/Editor.hpp"
#include "util/Util.hpp"
#include <QKeySequence>
#include <QShortcut>

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QTextEdit(parent),
      m_children({}),
      m_grid(nullptr)
{
    installEventFilter(this);
    setFont(Editor::getInstance()->font());
}

[[nodiscard]] bool
Buffer::eventFilter(QObject *obj, QEvent *event)
{
    auto editor = Editor::getInstance();
    if (editor->getMode() == util::Mode::Insert) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            editor->setMode(util::Mode::Normal);
        } else {
            keyPressEvent(keyEvent);
        }
        return true;
    } else {
        return editor->getInputHandler()->eventFilter(obj, event);
    }
}

} // namespace pico
