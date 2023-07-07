#include "TextEdit.hpp"
#include "editor/Editor.hpp"

#include <QKeyEvent>

using namespace Qt;
namespace pico {

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent),
      m_keyListener(parent)
{
    m_keyListener.addBinding({ Key_I }, Mode::Normal, [=]() {
        qDebug() << "BINDING HIT";
        Editor::getInstance()->setMode(Mode::Insert);
    });
}

void
TextEdit::keyPressEvent(QKeyEvent *event)
{
    auto editor = Editor::getInstance();

    if (editor->mode() != Mode::Insert)
        m_keyListener.handleKeyPress(event->key());
    // TODO its EventListen who handles escape
    else if (event->key() == Qt::Key_Escape)
        editor->setMode(Mode::Normal);
    else
        QTextEdit::keyPressEvent(event);
}

} // namespace pico
