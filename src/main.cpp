#include "MainWindow.hpp"
#include <QApplication>

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("org.pico.code");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Pico");
    QApplication::setApplicationDisplayName("Pico");

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
