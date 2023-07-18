#include "Editor.hpp"
#include <QApplication>
#include <QStackedLayout>
#include <QTextEdit>
#include <QTreeView>
#include <qnamespace.h>

#include "MainWindow.hpp"
#include "editor/TextEdit.hpp"
#include "extern/qlightterminal.h"
#include "util/Util.hpp"

/* for quick and short callbacks */
#define CALLBACK(X)                                                                               \
    ([=]() {                                                                                      \
        X;                                                                                        \
    })

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
    obj->installEventFilter(m_keyFilter);
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
    modeChange(mode);
}

Buffer *
Editor::currentBuffer(void)
{
    return static_cast<Buffer *>(m_stack->currentWidget());
}

void
Editor::addBuffer(QWidget *widget)
{
    m_stack->addWidget(new Buffer(widget, this));
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
    addBuffer(new TextEdit(this));

    addBinding({ CTRL | Key_B }, Mode::Normal, CALLBACK(prevBuffer()));
    addBinding({ CTRL | Key_N }, Mode::Normal, CALLBACK(nextBuffer()));
    addBinding({ CTRL | Key_B }, Mode::Terminal, CALLBACK(prevBuffer()));
    addBinding({ CTRL | Key_N }, Mode::Terminal, CALLBACK(nextBuffer()));

    addBinding({ Key_Space, Key_B, Key_N }, Mode::Normal, [=]() {
        addBuffer(new TextEdit(this));
    });
    addBinding({ CTRL | Key_Backslash }, Mode::Normal, [=]() {
        currentBuffer()->toggleTerminal();
    });
    addBinding({ CTRL | Key_Backslash }, Mode::Terminal, [=]() {
        currentBuffer()->toggleTerminal();
    });
    addBinding({ Key_Space, Key_V }, Mode::Normal, [=]() {
        currentBuffer()->splitLeft(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_H }, Mode::Normal, [=]() {
        currentBuffer()->splitBottom(new TextEdit(this));
    });
    addBinding({ Key_Space, Key_E }, Mode::Normal, [=]() {
        currentBuffer()->toggleFileTree();
    });
    addBinding({ Key_Space, Key_O }, Mode::Normal, [=]() {
        currentBuffer()->showFileTree();
    });
}

} // namespace pico
