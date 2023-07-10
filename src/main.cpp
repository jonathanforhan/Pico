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
    QApplication::setFont(QFont{ "JetBrains Mono NF", 11 });

    pico::MainWindow window;
    window.show();

    return app.exec();
}
