#include "Editor.hpp"

#include "util/Util.hpp"

namespace pico {

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this)),
      m_bufferStack(new QStackedLayout(this))
{
    setFont({ "JetBrains Mono NF", 12 });
}

void
Editor::Init(void) /* Init is an extension of the constructor see header */
{
    installEventFilter(m_inputHandler);

    setLayout(m_bufferStack);
    m_bufferStack->setSpacing(0);
    m_bufferStack->setContentsMargins(0, 0, 0, 0);

    Buffer *entryBuffer = new Buffer(this);
    m_bufferStack->addWidget(entryBuffer);
    entryBuffer->setPlaceholderText("Welcome to Pico Editor");

    m_inputHandler->addBinding({ Qt::Key_I }, util::Mode::Normal, [=]() {
        setMode(util::Mode::Insert);
    });

    m_inputHandler->addBinding({ Qt::CTRL | Qt::Key_H }, util::Mode::Normal, [=]() {
        prevBuffer();
    });

    m_inputHandler->addBinding({ Qt::CTRL | Qt::Key_L }, util::Mode::Normal, [=]() {
        nextBuffer();
    });

    m_inputHandler->addBinding({ Qt::Key_Space, Qt::Key_B, Qt::Key_N }, util::Mode::Normal, [=]() {
        addBuffer(new Buffer(this));
    });

    m_inputHandler->addBinding({ Qt::Key_Space, Qt::Key_B, Qt::Key_D }, util::Mode::Normal, [=]() {
        removeBuffer(getCurrentBuffer());
    });
}

Editor *
Editor::getInstance(QWidget *parent)
{
    static Editor *instance = nullptr;
    /* Generates instance on MainWindow creation */
    if (instance == nullptr) {
        instance = new Editor(parent);
        instance->Init();
    }
    return instance;
}

void
Editor::forwardEventFilter(QWidget *widget)
{
    widget->installEventFilter(m_inputHandler);
}

util::Mode
Editor::getMode(void) const
{
    return m_inputHandler->m_mode;
}

void
Editor::setMode(util::Mode mode)
{
    m_inputHandler->setMode(mode);
    m_bufferStack->currentWidget()->setFocus();
}

bool
Editor::isShiftPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.shift);
}

bool
Editor::isControlPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.control);
}

bool
Editor::isAltPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.alt);
}

void
Editor::nextBuffer(void)
{
    auto i = m_bufferStack->currentIndex();
    i = i == m_bufferStack->count() - 1 ? 0 : i + 1;
    m_bufferStack->setCurrentIndex(i);
}

void
Editor::prevBuffer(void)
{
    auto i = m_bufferStack->currentIndex();
    i = i == 0 ? m_bufferStack->count() - 1 : i - 1;
    m_bufferStack->setCurrentIndex(i);
}

void
Editor::nthBuffer(qsizetype i)
{
    if (i < 0 || i <= m_bufferStack->count())
        return;
    m_bufferStack->setCurrentIndex(i);
}

Buffer *
Editor::getCurrentBuffer(void)
{
    return static_cast<Buffer *>(m_bufferStack->currentWidget());
}

void
Editor::addBuffer(Buffer *buffer)
{
    m_bufferStack->addWidget(buffer);
    m_bufferStack->setCurrentWidget(buffer);
}

void
Editor::removeBuffer(Buffer *buffer)
{
    m_bufferStack->removeWidget(buffer);
}

} // namespace pico
