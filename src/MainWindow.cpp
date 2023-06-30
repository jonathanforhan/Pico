#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QScreen>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
{
    setCentralWidget(Editor::getInstance(this));
    Editor::getInstance()->Init();
}

QSize
MainWindow::sizeHint(void) const
{
    /* Application launches at 75% of max size */
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
