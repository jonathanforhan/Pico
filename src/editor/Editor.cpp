#include "Editor.hpp"
#include <QStackedLayout>
#include <QTextEdit>
#include <QTreeView>
#include <qapplication.h>
#include <qnamespace.h>

#include "MainWindow.hpp"
#include "editor/Buffer.hpp"
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
    obj->installEventFilter(&this->m_keyFilter);
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
    qDebug() << i;
}

void
Editor::prevBuffer(void)
{
    auto i = m_stack->currentIndex() - 1;
    if (i < 0)
        i = m_stack->count() - 1;
    m_stack->setCurrentIndex(i);
    qDebug() << i;
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
      m_keyFilter({}),
      m_stack(new QStackedLayout(this))
{
    parent->setFont({ "JetBrains Mono NF", 11 });
    m_stack->setSpacing(0);
    m_stack->setContentsMargins(0, 0, 0, 0);
}

void
Editor::Init(void)
{
    using namespace Qt;
    installEventFilter(&m_keyFilter);

    m_stack->addWidget(new Buffer(this));

    m_keyFilter.addBinding({ CTRL | Key_B }, Mode::Normal, [=]() {
        prevBuffer();
    });
    m_keyFilter.addBinding({ CTRL | Key_N }, Mode::Normal, [=]() {
        nextBuffer();
    });
    m_keyFilter.addBinding({ Key_Space, Key_B, Key_N }, Mode::Normal, [=]() {
        addBuffer();
    });
    m_keyFilter.addBinding({ Key_F, Key_R, Key_I, Key_E, Key_D }, Mode::Normal, [=]() {
        qDebug() << "FRIED";
    });

    /* example of remapping TODO better implementation */
    m_keyFilter.addBinding({ Key_J, Key_K }, Mode::Normal, [=]() {
        auto *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_I, Qt::NoModifier);
        QApplication::postEvent(QApplication::focusWidget(), (QEvent *)e);
    });
}

} // namespace pico
