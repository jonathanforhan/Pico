#include "FileTree.hpp"

#include <QFileSystemModel>
#include <QKeyEvent>

namespace pico {

FileTree::FileTree(QWidget *parent)
    : QTreeView(parent),
      PicoObject(this)
{
    auto fs = new QFileSystemModel(this);
    fs->setRootPath(QDir::currentPath());
    this->setModel(fs);
    this->setRootIndex(fs->index(QDir::currentPath()));
    this->setHeaderHidden(true);
    this->hideColumn(1);
    this->hideColumn(2);
    this->hideColumn(3);
    this->setMaximumWidth(400);
    this->setUniformRowHeights(true);
}

void
FileTree::keyPressEvent(QKeyEvent *event)
{
    using namespace Qt;
    auto key = event->key();

    switch (key) {
    case Key_H:
        return collapse(currentIndex());
    case Key_J:
        return setCurrentIndex(indexBelow(currentIndex()));
    case Key_K:
        return setCurrentIndex(indexAbove(currentIndex()));
    case Key_L:
        expand(currentIndex());
        if (isExpanded(currentIndex()))
            clicked(currentIndex());
        return;
    }
}

} // namespace pico
