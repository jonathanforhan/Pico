#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QScreen>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
      m_editor(new Editor(this))
{
    setCentralWidget(m_editor);
}

MainWindow::~MainWindow()
{}

bool
MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    /* Editor processes all key events */
    return m_editor->eventFilter(obj, event);
}

QSize
MainWindow::sizeHint(void) const
{
    // Default initial size if 75% screen W and H
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
