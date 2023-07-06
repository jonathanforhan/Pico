#include "Editor.hpp"

#include "MainWindow.hpp"
#include "editor/TextEdit.hpp"
#include "util/Util.hpp"

namespace pico {

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this)),
      m_bufferStack(new QStackedLayout(this))
{
    auto font = QFont{ "JetBrains Mono NF" };
    font.setPixelSize(15);
    setFont(font);
}

void
Editor::Init(void) /* Init is an extension of the constructor see header */
{
    using namespace Qt;
    using namespace util;
    m_bufferStack->setSpacing(0);
    m_bufferStack->setContentsMargins(0, 0, 0, 0);

    addBuffer(new Buffer(this));

    addBinding({ Key_I }, Mode::Normal, [=]() {
        setMode(Mode::Insert);
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

MainWindow *
Editor::getMainWindow(void)
{
    return static_cast<MainWindow *>(parent());
}

void
Editor::forwardEventFilter(QWidget *widget)
{
    widget->installEventFilter(m_inputHandler);
}

void
Editor::addBinding(QList<QKeyCombination> keys, util::Mode mode, const std::function<void()> &fn)
{
    m_inputHandler->addBinding(keys, mode, fn);
}

util::Mode
Editor::mode(void) const
{
    return m_inputHandler->m_mode;
}

void
Editor::setMode(util::Mode mode)
{
    m_inputHandler->setMode(mode);
}

bool
Editor::isShiftPressed(void)
{
    return m_inputHandler->m_modifiers.shift;
}

bool
Editor::isControlPressed(void)
{
    return m_inputHandler->m_modifiers.control;
}

bool
Editor::isAltPressed(void)
{
    return m_inputHandler->m_modifiers.alt;
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
