#include "TextEdit.hpp"
#include "editor/Editor.hpp"

#include <QKeyEvent>

using namespace Qt;
namespace pico {

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent),
      PicoObject(this)
{
    auto editor = Editor::getInstance();

    addBinding({ Key_I }, Mode::Normal, [=]() {
        editor->setMode(Mode::Insert);
    });
}

void
TextEdit::keyPressEvent(QKeyEvent *event)
{
    auto editor = Editor::getInstance();

    if (editor->mode() != Mode::Insert)
        handleKeyPress(event->key());
    else
        QTextEdit::keyPressEvent(event);
}

} // namespace pico
