#include "editor/Buffer.hpp"

#include <QDir>
#include <QDockWidget>

#include "MainWindow.hpp"
#include "editor/Editor.hpp"
#include "editor/TextEdit.hpp"

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QWidget(parent),
      m_dockState({}),
      m_grid(new QGridLayout(this)),
      m_fsModel(new QFileSystemModel(this)),
      m_fileTree(new QTreeView(this))
{
    auto editor = Editor::getInstance();
    editor->forwardEventFilter(this);
    editor->forwardEventFilter(m_fileTree);
    auto mainWindow = editor->getMainWindow();

    connect(this, &Buffer::setDockWidget, [=](Qt::DockWidgetArea area, QWidget *widget) {
        mainWindow->setDockWidget(area, widget);
    });
    connect(this, &Buffer::toggleDock, [=](Qt::DockWidgetArea area) {
        mainWindow->toggleDock(area);
        toggleDockState(area);
    });
    connect(this, &Buffer::hideDock, [=](Qt::DockWidgetArea area) {
        mainWindow->hideDock(area);
        setDockState(area, false);
    });
    connect(this, &Buffer::showDock, [=](Qt::DockWidgetArea area) {
        mainWindow->showDock(area);
        setDockState(area, true);
    });

    m_grid->setSpacing(0);
    m_grid->setContentsMargins(0, 0, 0, 0);

    m_fsModel->setRootPath(QDir::currentPath());
    m_fileTree->setModel(m_fsModel);
    m_fileTree->setRootIndex(m_fsModel->index(QDir::currentPath()));
    m_fileTree->setHeaderHidden(true);
    m_fileTree->hideColumn(1);
    m_fileTree->hideColumn(2);
    m_fileTree->hideColumn(3);

    setDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_fileTree);

    m_grid->addWidget(new TextEdit(this), 0, 0);
}

void
Buffer::updateDocks(void)
{
    auto mainWindow = Editor::getInstance()->getMainWindow();
    mainWindow->hideAllDocks();
    if (m_dockState.left)
        mainWindow->showDock(Qt::DockWidgetArea::LeftDockWidgetArea);
    if (m_dockState.right)
        mainWindow->showDock(Qt::DockWidgetArea::RightDockWidgetArea);
    if (m_dockState.bottom)
        mainWindow->showDock(Qt::DockWidgetArea::BottomDockWidgetArea);
}

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
