#include "Editor.hpp"
#include <QApplication>
#include <QStackedLayout>
#include <QTextEdit>
#include <QTreeView>

#include "MainWindow.hpp"
#include "editor/TextEdit.hpp"
#include "util/Util.hpp"

namespace pico {

Editor *
Editor::getInstance(QMainWindow *parent)
{
    static Editor *instance = nullptr;
    if (instance == nullptr) {
        instance = new Editor(parent);
        instance->Init();
    }
    return instance;
}

void
Editor::forwardKeyFilter(QObject *obj)
{
    obj->installEventFilter(this->m_keyFilter);
}

bool
Editor::addBinding(QList<QKeyCombination> keyCombo, Mode mode, std::function<void()> callback)
{
    return m_keyFilter->addBinding(keyCombo, mode, callback);
}

bool
Editor::shiftState(void)
{
    return m_modifiers.shift;
}

bool
Editor::controlState(void)
{
    return m_modifiers.control;
}

bool
Editor::altState(void)
{
    return m_modifiers.alt;
}

Mode
Editor::mode(void)
{
    return m_mode;
}

void
Editor::setMode(Mode mode)
{
    m_mode = mode;
}

Buffer *
Editor::currentBuffer(void)
{
    return static_cast<Buffer *>(m_stack->currentWidget());
}

void
Editor::addBuffer(void)
{
    m_stack->addWidget(new Buffer(this));
}

void
Editor::nextBuffer(void)
{
    auto i = m_stack->currentIndex() + 1;
    if (i >= m_stack->count())
        i = 0;
    m_stack->setCurrentIndex(i);
}

void
Editor::prevBuffer(void)
{
    auto i = m_stack->currentIndex() - 1;
    if (i < 0)
        i = m_stack->count() - 1;
    m_stack->setCurrentIndex(i);
}

void
Editor::nthBuffer(int index)
{
    if (index < 0 || index >= m_stack->count())
        return;
    m_stack->setCurrentIndex(index);
}

Editor::Editor(QMainWindow *parent)
    : QWidget(parent),
      m_modifiers({}),
      m_mode(Mode::Normal),
      m_keyFilter(nullptr),
      m_stack(new QStackedLayout(this))
{
    parent->setFont({ "JetBrains Mono NF", 11 });
    m_stack->setSpacing(0);
    m_stack->setContentsMargins(0, 0, 0, 0);
}

void
Editor::Init(void)
{
    /* initialize editor-instance dependent variables */
    m_keyFilter = new KeyFilter(this);

    using namespace Qt;
    installEventFilter(m_keyFilter);
    m_stack->addWidget(new Buffer(this));

    addBinding({ CTRL | Key_B }, Mode::Normal, [=]() {
        prevBuffer();
    });
    addBinding({ CTRL | Key_N }, Mode::Normal, [=]() {
        nextBuffer();
    });
    addBinding({ Key_Space, Key_B, Key_N }, Mode::Normal, [=]() {
        addBuffer();
    });
    addBinding({ Key_Space, Key_V }, Mode::Normal, [=]() {
        currentBuffer()->splitLeft(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_V | SHIFT }, Mode::Normal, [=]() {
        currentBuffer()->splitRight(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_H }, Mode::Normal, [=]() {
        currentBuffer()->splitTop(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_H | SHIFT }, Mode::Normal, [=]() {
        currentBuffer()->splitBottom(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_E }, Mode::Normal, [=]() {
        currentBuffer()->toggleFileTree();
    });
    addBinding({ Key_Space, Key_O }, Mode::Normal, [=]() {
        currentBuffer()->showFileTree();
    });

    /* example of remapping TODO better implementation */
    addBinding({ Key_J, Key_K }, Mode::Normal, [=]() {
        auto *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_I, Qt::NoModifier);
        QApplication::postEvent(QApplication::focusWidget(), (QEvent *)e);
    });
}

} // namespace pico
