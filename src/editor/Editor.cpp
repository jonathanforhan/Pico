#include "Editor.hpp"
#include "MainWindow.hpp"
#include "editor/TextEdit.hpp"
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

bool
Editor::shiftState(void)
{
    return m_modState.shift;
}

bool
Editor::controlState(void)
{
    return m_modState.shift;
}

bool
Editor::altState(void)
{
    return m_modState.shift;
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
      m_modState({}),
      m_mode(Mode::Normal)
{
    parent->setFont({ "JetBrains Mono NF", 11 });
    auto layout = new QHBoxLayout(this);
    layout->addWidget(new TextEdit(this));
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
}

constexpr void
Editor::Init(void)
{}

} // namespace pico
