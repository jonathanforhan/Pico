#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QMenuBar>
#include <QScreen>

#include <editor/Editor.hpp>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
      m_leftDock(new QDockWidget()),
      m_rightDock(new QDockWidget()),
      m_bottomDock(new QDockWidget()),
      m_floatingDock(new QDockWidget())
{
    setCentralWidget(Editor::getInstance(this));
    setFont(Editor::getInstance()->font());

    m_leftDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_leftDock->setTitleBarWidget(new QWidget());
    m_rightDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_rightDock->setTitleBarWidget(new QWidget());
    m_bottomDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    m_bottomDock->setTitleBarWidget(new QWidget());

    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_leftDock);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_rightDock);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_bottomDock);
    hideAllDocks();
}

void
MainWindow::setDockWidget(Qt::DockWidgetArea area, QWidget *widget)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        return m_leftDock->setWidget(widget);
    case Qt::DockWidgetArea::RightDockWidgetArea:
        return m_rightDock->setWidget(widget);
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        return m_bottomDock->setWidget(widget);
    case Qt::DockWidgetArea::NoDockWidgetArea:
        return m_floatingDock->setWidget(widget);
    default:
        return;
    }
}

void
MainWindow::hideAllDocks(void)
{
    m_leftDock->hide();
    m_rightDock->hide();
    m_bottomDock->hide();
    m_floatingDock->hide();
}

void
MainWindow::hideDock(Qt::DockWidgetArea area)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        return m_leftDock->hide();
    case Qt::DockWidgetArea::RightDockWidgetArea:
        return m_rightDock->hide();
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        return m_bottomDock->hide();
    case Qt::DockWidgetArea::NoDockWidgetArea:
        return m_floatingDock->hide();
    default:
        return;
    }
}

void
MainWindow::showAllDocks(void)
{
    m_leftDock->show();
    m_rightDock->show();
    m_bottomDock->show();
    m_floatingDock->show();
}

void
MainWindow::showDock(Qt::DockWidgetArea area)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        m_leftDock->show();
        return m_leftDock->widget()->setFocus();
    case Qt::DockWidgetArea::RightDockWidgetArea:
        m_rightDock->show();
        return m_rightDock->widget()->setFocus();
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        m_bottomDock->show();
        return m_bottomDock->widget()->setFocus();
    case Qt::DockWidgetArea::NoDockWidgetArea:
        m_floatingDock->show();
        return m_floatingDock->widget()->setFocus();
    default:
        return;
    }
}

void
MainWindow::toggleDock(Qt::DockWidgetArea area)
{
    switch (area) {
    case Qt::DockWidgetArea::LeftDockWidgetArea:
        if (m_leftDock->isHidden()) {
            m_leftDock->show();
            return m_leftDock->widget()->setFocus();
        } else {
            return m_leftDock->hide();
        }
    case Qt::DockWidgetArea::RightDockWidgetArea:
        if (m_rightDock->isHidden()) {
            m_rightDock->show();
            return m_rightDock->widget()->setFocus();
        } else {
            return m_rightDock->hide();
        }
    case Qt::DockWidgetArea::BottomDockWidgetArea:
        if (m_bottomDock->isHidden()) {
            m_bottomDock->show();
            return m_bottomDock->widget()->setFocus();
        } else {
            return m_bottomDock->hide();
        }
    case Qt::DockWidgetArea::NoDockWidgetArea:
        if (m_floatingDock->isHidden()) {
            m_floatingDock->show();
            return m_floatingDock->widget()->setFocus();
        } else {
            return m_floatingDock->hide();
        }
    default:
        return;
    }
}

QSize
MainWindow::sizeHint(void) const
{
    /* Application launches at 75% of max size */
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
