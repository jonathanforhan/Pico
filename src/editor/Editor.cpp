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

    connect(m_bufferStack, &QStackedLayout::currentChanged, [=]() {
        ((Buffer *)m_bufferStack->currentWidget())->updateDocks();
    });

    addBuffer(new Buffer(this));

    addBinding({ Key_I }, Mode::Normal, [=]() {
        setMode(Mode::Insert);
    });

    addBinding({ Key_H }, Mode::Normal, [=]() {
        // TODO better way to access buffer children
        TextEdit *edit = static_cast<TextEdit *>(getCurrentBuffer()->getChildAtPosition(0, 0));
        edit->moveCursor(QTextCursor::MoveOperation::Left);
    });

    addBinding({ Key_J }, Mode::Normal, [=]() {
        TextEdit *edit = static_cast<TextEdit *>(getCurrentBuffer()->getChildAtPosition(0, 0));
        edit->moveCursor(QTextCursor::MoveOperation::Down);
    });

    addBinding({ Key_K }, Mode::Normal, [=]() {
        TextEdit *edit = static_cast<TextEdit *>(getCurrentBuffer()->getChildAtPosition(0, 0));
        edit->moveCursor(QTextCursor::MoveOperation::Up);
    });

    addBinding({ Key_L }, Mode::Normal, [=]() {
        TextEdit *edit = static_cast<TextEdit *>(getCurrentBuffer()->getChildAtPosition(0, 0));
        edit->moveCursor(QTextCursor::MoveOperation::Right);
    });

    addBinding({ CTRL | Key_H }, Mode::Normal, [=]() {
        prevBuffer();
    });

    addBinding({ CTRL | Key_L }, Mode::Normal, [=]() {
        nextBuffer();
    });

    addBinding({ Key_Space, Key_B, Key_N }, Mode::Normal, [=]() {
        addBuffer(new Buffer(this));
    });

    addBinding({ Key_Space, Key_B, Key_D }, Mode::Normal, [=]() {
        removeBuffer(getCurrentBuffer());
    });

    addBinding({ Key_Space, Key_E }, Mode::Normal, [=]() {
        getCurrentBuffer()->toggleDock(Qt::DockWidgetArea::LeftDockWidgetArea);
    });

    addBinding({ CTRL | Key_Backslash }, Mode::Normal, [=]() {
        getCurrentBuffer()->toggleDock(Qt::DockWidgetArea::BottomDockWidgetArea);
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

inline void
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

inline void
Editor::nextBuffer(void)
{
    auto i = m_bufferStack->currentIndex();
    i = i == m_bufferStack->count() - 1 ? 0 : i + 1;
    m_bufferStack->setCurrentIndex(i);
}

inline void
Editor::prevBuffer(void)
{
    auto i = m_bufferStack->currentIndex();
    i = i == 0 ? m_bufferStack->count() - 1 : i - 1;
    m_bufferStack->setCurrentIndex(i);
}

inline void
Editor::nthBuffer(qsizetype i)
{
    if (i < 0 || i <= m_bufferStack->count())
        return;
    m_bufferStack->setCurrentIndex(i);
}

inline Buffer *
Editor::getCurrentBuffer(void)
{
    return static_cast<Buffer *>(m_bufferStack->currentWidget());
}

inline void
Editor::addBuffer(Buffer *buffer)
{
    m_bufferStack->addWidget(buffer);
    m_bufferStack->setCurrentWidget(buffer);
}

inline void
Editor::removeBuffer(Buffer *buffer)
{
    m_bufferStack->removeWidget(buffer);
}

} // namespace pico
