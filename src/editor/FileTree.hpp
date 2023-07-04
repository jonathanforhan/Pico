#pragma once

#include <QKeyEvent>
#include <QTreeView>

namespace pico {

class FileTree : public QTreeView
{
    Q_OBJECT

public:
    explicit FileTree(QWidget *parent = nullptr);

private:
    void
    keyPressEvent(QKeyEvent *event) override;
};

} // namespace pico
