#include "MainWindow.hpp"

#include <QGuiApplication>
#include <QScreen>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent),
      m_editor(Editor::getInstance(this))
{
    this->setCentralWidget(m_editor);
    installEventFilter((QObject *)m_editor->getInputHandler());
}

QSize
MainWindow::sizeHint(void) const
{
    /* Application launches at 75% of max size */
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
