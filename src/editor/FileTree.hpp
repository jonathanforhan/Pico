#pragma once

#include <QTreeView>

#include "editor/PicoWidget.hpp"

namespace pico {

class FileTree : public QTreeView, public PicoWidget
{
    Q_OBJECT

public:
    explicit FileTree(QWidget *parent);

protected:
    void
    keyPressEvent(QKeyEvent *event) override;

private:
};

} // namespace pico
