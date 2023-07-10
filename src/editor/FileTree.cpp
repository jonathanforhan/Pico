#include "FileTree.hpp"

#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QKeyEvent>

namespace pico {

FileTree::FileTree(QWidget *parent)
    : QTreeView(parent),
      PicoWidget(this)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto fs = new QFileSystemModel(this);
    fs->setRootPath(QDir::currentPath());
    fs->setOption(QFileSystemModel::DontWatchForChanges);
    fs->setOption(QFileSystemModel::DontUseCustomDirectoryIcons);
    setModel(fs);
    setRootIndex(fs->index(QDir::currentPath()));
    setHeaderHidden(true);
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
    setMaximumWidth(400);
    setUniformRowHeights(true);
    setSortingEnabled(false);
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
