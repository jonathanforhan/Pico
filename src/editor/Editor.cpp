#include "Editor.hpp"

#include "editor/TextBuffer.hpp"
#include "util/Util.hpp"

namespace pico {

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this)),
      m_bufferStack(new BufferStack(this)),
      m_font("JetBrains Mono NF", 12)
{}

void
Editor::Init(void)
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
    /* Generates instance on MainWindow creation */
    static Editor *instance = new Editor(parent);
    return instance;
}

InputHandler *
Editor::getInputHandler(void)
{
    return m_inputHandler;
}

Mode
Editor::getMode(void) const
{
    return m_inputHandler->getMode();
}

void
Editor::setMode(Mode mode)
{
    m_inputHandler->setMode(mode);
    m_bufferStack->currentWidget()->setFocus();
}

const QFont &
Editor::getFont(void) const
{
    return m_font;
}

void
Editor::setFont(QFont &font)
{
    m_font = font;
}

} // namespace pico
