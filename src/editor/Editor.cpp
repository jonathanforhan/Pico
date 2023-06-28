#include "Editor.hpp"

#include <QStackedLayout>

#include "editor/TextBuffer.hpp"

namespace pico {

Editor *Editor::s_instance = nullptr;

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this))
{
    auto stack = new QStackedLayout;
    setLayout(stack);
    stack->setSpacing(0);
    stack->setContentsMargins(0, 0, 0, 0);

    TextBuffer *textEdit = new TextBuffer(this);
    textEdit->installEventFilter(m_inputHandler);

    TextBuffer *textEdit2 = new TextBuffer(this);
    textEdit2->installEventFilter(m_inputHandler);

    stack->addWidget(textEdit);
    stack->addWidget(textEdit2);

    m_inputHandler->addBinding({ Qt::Key_Space, Qt::Key_J, Qt::Key_K }, [=]() {
        qDebug() << "Actually fucking works";
    });

    m_inputHandler->addBinding({ Qt::Key_S | binding::Mod::Shift | binding::Mod::Control }, [=]() {
        qDebug() << "Saving Ctrl-Shft-S";
    });
}

Editor *
Editor::getInstance(QWidget *w)
{
    if (s_instance == nullptr)
        s_instance = new Editor(w);
    return s_instance;
}

const InputHandler *
Editor::getInputHandler()
{
    return m_inputHandler;
}

} // namespace pico
