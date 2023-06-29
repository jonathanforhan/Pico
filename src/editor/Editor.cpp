#include "Editor.hpp"

#include "editor/TextBuffer.hpp"
#include "util/Util.hpp"

namespace pico {

Editor *Editor::s_instance = nullptr;

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this)),
      m_bufferStack(new BufferStack(this))
{
    setLayout(m_bufferStack);
    m_bufferStack->setSpacing(0);
    m_bufferStack->setContentsMargins(0, 0, 0, 0);

    TextBuffer *entryBuffer = new TextBuffer(this);
    m_bufferStack->addWidget(entryBuffer);
    entryBuffer->append("Welcome to Pico Editor");

    m_inputHandler->addBinding({ Key::H | ModKey::Control }, Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == 0 ? m_bufferStack->count() - 1 : i - 1;
        m_bufferStack->setCurrentIndex(i);
    });

    m_inputHandler->addBinding({ Key::L | ModKey::Control }, Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == m_bufferStack->count() - 1 ? 0 : i + 1;
        m_bufferStack->setCurrentIndex(i);
    });

    m_inputHandler->addBinding({ Key::Space, Key::B, Key::N }, Mode::Normal, [=]() {
        auto edit = new TextBuffer(this);
        m_bufferStack->addWidget(edit);
        m_bufferStack->setCurrentWidget(edit);
    });

    m_inputHandler->addBinding({ Key::Space, Key::B, Key::D }, Mode::Normal, [=]() {
        m_bufferStack->removeWidget(m_bufferStack->currentWidget());
    });

    m_inputHandler->addBinding({ Key::I }, Mode::Normal, [=]() {
        setMode(Mode::Insert);
    });

    m_inputHandler->addBinding({ Qt::Key_Escape }, Mode::Insert, [=]() {
        setMode(Mode::Normal);
    });
}

Editor *
Editor::getInstance(QWidget *parent)
{
    if (s_instance == nullptr)
        s_instance = new Editor(parent);
    return s_instance;
}

InputHandler *
Editor::getInputHandler(void)
{
    return m_inputHandler;
}

Mode
Editor::getMode(void)
{
    return m_inputHandler->getMode();
}

void
Editor::setMode(Mode mode)
{
    m_inputHandler->setMode(mode);
    m_bufferStack->currentWidget()->setFocus();
}

} // namespace pico
