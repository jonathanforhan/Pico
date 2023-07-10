#pragma once

#include <QBoxLayout>
#include <QSplitter>
#include <QTreeView>
#include <QWidget>

#include <editor/TextEdit.hpp>

namespace pico {

class Buffer : public QWidget
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

    QSplitter *
    splitter();

    void
    splitLeft(QWidget *widget);

    void
    splitRight(QWidget *widget);

    void
    splitTop(QWidget *widget);

    void
    splitBottom(QWidget *widget);

    QTreeView *
    fileTree();

    void
    setFileTree(QTreeView *fileTree);

    void
    showFileTree(void);

    void
    hideFileTree(void);

    void
    toggleFileTree(void);

private:
    QBoxLayout *m_layout;
    QSplitter *m_splitter;
    QTreeView *m_fileTree;
};

} // namespace pico
