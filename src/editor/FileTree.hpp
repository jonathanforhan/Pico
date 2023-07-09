#pragma once

#include <QTreeView>

#include "editor/PicoObject.hpp"

namespace pico {

class FileTree : public QTreeView, public PicoObject
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
