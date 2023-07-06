#include "editor/Buffer.hpp"

#include <QDir>
#include <QDockWidget>

#include "MainWindow.hpp"
#include "editor/Editor.hpp"
#include "editor/TextEdit.hpp"
#include "extern/qlightterminal.h"

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QWidget(parent),
      m_dockState({}),
      m_grid(new QGridLayout(this)),
      m_fsModel(new QFileSystemModel(this)),
      m_fileTree(new FileTree(this))
{
    auto editor = Editor::getInstance();
    editor->forwardEventFilter(this);

    auto textEdit = new TextEdit(this);

    m_grid->setSpacing(0);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->addWidget(textEdit, 0, 0);

    textEdit->setFocus();
}

void
Buffer::updateDocks(void)
{}

QWidget *
Buffer::getChildAtPosition(int row, int col)
{
    return m_grid->itemAtPosition(row, col)->widget();
}

void
Buffer::setDockState(Qt::DockWidgetArea area, bool state)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        m_dockState.left = state;
        break;
    case Qt::DockWidgetArea::RightDockWidgetArea:
        m_dockState.right = state;
        break;
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        m_dockState.bottom = state;
        break;
    default:
        return;
    }
}
void
Buffer::toggleDockState(Qt::DockWidgetArea area)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        m_dockState.left = !m_dockState.left;
        break;
    case Qt::DockWidgetArea::RightDockWidgetArea:
        m_dockState.right = !m_dockState.right;
        break;
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        m_dockState.bottom = !m_dockState.bottom;
        break;
    default:
        return;
    }
}

} // namespace pico
