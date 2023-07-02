#pragma once

#include <QDockWidget>
#include <QMainWindow>

namespace pico {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = nullptr);

public slots:
    void
    setDockWidget(Qt::DockWidgetArea area, QWidget *widget);

    void
    hideAllDocks(void);

    void
    hideDock(Qt::DockWidgetArea area);

    void
    showAllDocks(void);

    void
    showDock(Qt::DockWidgetArea area);

    void
    toggleDock(Qt::DockWidgetArea area);

private:
    QSize
    sizeHint(void) const override;

private:
    QDockWidget *m_leftDock;
    QDockWidget *m_rightDock;
    QDockWidget *m_bottomDock;
    QDockWidget *m_floatingDock;
};

} // namespace pico
