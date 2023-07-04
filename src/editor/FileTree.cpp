#include "FileTree.hpp"

namespace pico {

FileTree::FileTree(QWidget *parent)
    : QTreeView(parent)
{}

void
FileTree::keyPressEvent(QKeyEvent *event)
{
    using namespace Qt;
    if (!this->hasFocus())
        return;

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
        if (!isExpanded(currentIndex()))
            clicked(currentIndex());
        break;
    default:
        return;
    }
}

} // namespace pico
