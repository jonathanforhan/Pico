#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QScreen>

#include <editor/Editor.hpp>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
      m_leftDock(new QDockWidget()),
      m_rightDock(new QDockWidget()),
      m_bottomDock(new QDockWidget())
{
    setCentralWidget(Editor::getInstance(this));
    setFont(Editor::getInstance()->font());

    // m_leftDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // m_leftDock->setTitleBarWidget(new QWidget());
    // m_rightDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // m_rightDock->setTitleBarWidget(new QWidget());
    // m_bottomDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // m_bottomDock->setTitleBarWidget(new QWidget());
    //
    // addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, m_leftDock);
    // addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_rightDock);
    // addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, m_bottomDock);
}

QSize
MainWindow::sizeHint(void) const
{
    /* Application launches at 75% of max size */
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
