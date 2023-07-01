#include "Editor.hpp"

#include "editor/Buffer.hpp"
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
    entryBuffer->append("Welcome to Pico Editor");

    m_inputHandler->addBinding({ Qt::Key_I }, util::Mode::Normal, [=]() {
        setMode(util::Mode::Insert);
    });

    m_inputHandler->addBinding({ Qt::Key_Escape }, util::Mode::Insert, [=]() {
        setMode(util::Mode::Normal);
    });

    m_inputHandler->addBinding({ Qt::Key_A }, util::Mode::Insert, [=]() {
        ((Buffer *)m_bufferStack->currentWidget())->insertPlainText("a");
    });

    m_inputHandler->addBinding({ Qt::CTRL | Qt::Key_H }, util::Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == 0 ? m_bufferStack->count() - 1 : i - 1;
        m_bufferStack->setCurrentIndex(i);
    });

    m_inputHandler->addBinding({ Qt::CTRL | Qt::Key_L }, util::Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == m_bufferStack->count() - 1 ? 0 : i + 1;
        m_bufferStack->setCurrentIndex(i);
    });

    m_inputHandler->addBinding({ Qt::Key_Space, Qt::Key_B, Qt::Key_N }, util::Mode::Normal, [=]() {
        auto edit = new Buffer(this);
        m_bufferStack->addWidget(edit);
        m_bufferStack->setCurrentWidget(edit);
    });

    m_inputHandler->addBinding({ Qt::Key_Space, Qt::Key_B, Qt::Key_D }, util::Mode::Normal, [=]() {
        m_bufferStack->removeWidget(m_bufferStack->currentWidget());
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

inline util::Mode
Editor::getMode(void) const
{
    return m_inputHandler->m_mode;
}

inline void
Editor::setMode(util::Mode mode)
{
    m_inputHandler->m_mode = mode;
    m_bufferStack->currentWidget()->setFocus();
}

inline bool
Editor::isShiftPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.shift);
}

inline bool
Editor::isControlPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.control);
}

inline bool
Editor::isAltPressed(void)
{
    return static_cast<bool>(m_inputHandler->m_modifiers.alt);
}

} // namespace pico
