#pragma once

#include <QDockWidget>
#include <QMainWindow>

namespace pico {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = nullptr);

private:
    QSize
    sizeHint(void) const override;

private:
    QDockWidget *m_leftDock;
    QDockWidget *m_rightDock;
    QDockWidget *m_bottomDock;
};

} // namespace pico
