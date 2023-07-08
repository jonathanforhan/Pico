#include "Editor.hpp"
#include "MainWindow.hpp"
#include "editor/TextEdit.hpp"
#include "util/Util.hpp"
#include <QBoxLayout>
#include <QTextEdit>
#include <QTreeView>

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
    return m_modifiers.shift;
}

bool
Editor::altState(void)
{
    return m_modifiers.shift;
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

Editor::Editor(QMainWindow *parent)
    : QWidget(parent),
      m_modifiers({}),
      m_mode(Mode::Normal),
      m_keyFilter({})
{
    parent->setFont({ "JetBrains Mono NF", 11 });
}

void
Editor::Init(void)
{
    using namespace Qt;
    auto layout = new QHBoxLayout(this);
    layout->addWidget(new TextEdit(this));
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    installEventFilter(&m_keyFilter);

    m_keyFilter.addBinding({ Key_J, Key_K }, Mode::Normal, [=]() {
        qDebug() << "J WOW";
    });
}

} // namespace pico
