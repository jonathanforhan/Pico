#include "MainWindow.hpp"
#include "editor/CommandPrompt.hpp"
#include "editor/Editor.hpp"
#include "util/Util.hpp"

#include <QApplication>
#include <QScreen>
#include <qnamespace.h>

namespace pico {

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
{
    auto *editor = Editor::getInstance(this);
    setCentralWidget(editor);
    auto *commandPromptDock = new QDockWidget(this);
    auto *commandPrompt = new CommandPrompt(this);
    commandPromptDock->setWidget(commandPrompt);
    addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, commandPromptDock);
    commandPromptDock->setTitleBarWidget(new QWidget(this));
    commandPromptDock->hide();

    editor->addBinding({ Qt::SHIFT | Qt::Key_Colon }, Mode::Normal, [=]() {
        editor->setMode(Mode::Command);
    });

    connect(editor, &Editor::modeChange, [=](Mode mode) {
        if (mode == Mode::Command) {
            commandPromptDock->show();
            commandPrompt->setFocus();
        } else {
            commandPromptDock->hide();
        }
    });

    /* hide the three dots on handle */
    QPalette pal = QApplication::palette(this);
    QString wcolor = pal.window().color().name();
    QString style = QString("QMainWindow::separator { background: %1;}").arg(wcolor);
    this->setStyleSheet(style);
}

QSize
MainWindow::sizeHint(void) const
{
    /* Application launches at 75% of max size */
    const auto rect = QGuiApplication::primaryScreen()->geometry();
    return QSize(rect.width(), rect.height()) * 0.75;
}

} // namespace pico
