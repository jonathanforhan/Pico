#pragma once

#include <QFileSystemModel>
#include <QGridLayout>
#include <QTextEdit>
#include <QTreeView>
#include <vector>

namespace pico {

class Buffer : public QWidget
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

    QWidget *
    getChildAtPosition(int row, int col);

    void
    updateDocks(void);

signals:
    void
    setDockWidget(Qt::DockWidgetArea area, QWidget *widget);

    void
    toggleDock(Qt::DockWidgetArea area);

    void
    showDock(Qt::DockWidgetArea area);

    void
    hideDock(Qt::DockWidgetArea area);

private:
    void
    setDockState(Qt::DockWidgetArea area, bool state);

    void
    toggleDockState(Qt::DockWidgetArea area);

private:
    struct {
        bool left;
        bool right;
        bool bottom;
    } m_dockState;
    QGridLayout *m_grid;
    QFileSystemModel *m_fsModel;
    QTreeView *m_fileTree;
};

} // namespace pico
