#pragma once

#include <QMainWindow>

#include <editor/Editor.hpp>

namespace pico {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QMainWindow *parent = nullptr);

private:
    QSize
    sizeHint(void) const override;

    Editor *m_editor;
};

} // namespace pico
