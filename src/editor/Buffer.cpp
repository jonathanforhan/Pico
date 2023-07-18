#include "Buffer.hpp"
#include "editor/FileTree.hpp"

#include <QApplication>

namespace pico {

Buffer::Buffer(QWidget *widget, QWidget *parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this)),
      m_splitter(new QSplitter(this)),
      m_fileTree(new FileTree(m_splitter)),
      m_terminal(new QLightTerminal(m_splitter))
{
    m_fileTree->hide();
    m_terminal->hide();
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_splitter);
    m_splitter->setHandleWidth(0);

    m_splitter->addWidget(widget);

    connect(m_fileTree, &FileTree::clicked, [=]() {

    });
}

QSplitter *
Buffer::splitter()
{
    return m_splitter;
}

void
Buffer::splitLeft(QWidget *widget)
{
    auto *oldWidget = (QWidget *)QApplication::focusWidget()->parent();
    QWidget *oldSplitter;

    if (oldWidget == m_splitter) {
        m_layout->removeWidget(m_splitter);
        oldSplitter = m_splitter;
        m_splitter = new QSplitter(this);
        m_splitter->setHandleWidth(0);
        m_splitter->addWidget(widget);
        m_splitter->addWidget(oldSplitter);
        m_layout->addWidget(m_splitter);

    } else {
        auto *parentSplitter = (QSplitter *)oldWidget->parent();

        int i = parentSplitter->indexOf(oldWidget);
        if (i == -1)
            return;

        oldSplitter = parentSplitter->widget(i);
        auto *newSplitter = new QSplitter(this);
        newSplitter->setHandleWidth(0);

        parentSplitter->replaceWidget(i, newSplitter);
        newSplitter->addWidget(widget);
        newSplitter->addWidget(oldSplitter);
    }
    m_fileTree->setParent(m_splitter);
    widget->setFocus();
}

void
Buffer::splitRight(QWidget *widget)
{
    // TODO
    splitLeft(widget);
}

void
Buffer::splitTop(QWidget *)
{
    // TODO
    throw;
}

void
Buffer::splitBottom(QWidget *widget)
{
    auto *oldWidget = (QWidget *)QApplication::focusWidget()->parent();
    QWidget *oldSplitter;

    if (oldWidget == m_splitter) {
        m_layout->removeWidget(m_splitter);
        oldSplitter = m_splitter;
        m_splitter = new QSplitter(Qt::Vertical, this);
        m_splitter->setHandleWidth(0);
        m_splitter->addWidget(oldSplitter);
        m_splitter->addWidget(widget);
        m_layout->addWidget(m_splitter);

    } else {
        auto *parentSplitter = (QSplitter *)oldWidget->parent();
        int i = parentSplitter->indexOf(oldWidget);
        if (i == -1)
            return;

        oldSplitter = parentSplitter->widget(i);
        auto *newSplitter = new QSplitter(Qt::Vertical, this);
        newSplitter->setHandleWidth(0);

        parentSplitter->replaceWidget(i, newSplitter);
        newSplitter->addWidget(oldSplitter);
        newSplitter->addWidget(widget);
    }
    m_fileTree->setParent(m_splitter);
    widget->setFocus();
}

QTreeView *
Buffer::fileTree()
{
    return m_fileTree;
}

void
Buffer::setFileTree(QTreeView *fileTree)
{
    delete m_fileTree;
    m_fileTree = fileTree;
}

void
Buffer::showFileTree(void)
{
    m_fileTree->show();
    m_fileTree->setFocus();
}

void
Buffer::hideFileTree(void)
{
    m_fileTree->hide();
}

void
Buffer::toggleFileTree(void)
{
    if (m_fileTree->isHidden()) {
        showFileTree();
        m_fileTree->setFocus();
    } else {
        hideFileTree();
    }
}

void
Buffer::showTerminal(void)
{
    m_terminal->show();
    m_terminal->setFocus();
}

void
Buffer::hideTerminal(void)
{
    m_terminal->hide();
}

void
Buffer::toggleTerminal(void)
{
    if (m_terminal->isHidden()) {
        showTerminal();
        m_terminal->setFocus();
    } else {
        hideTerminal();
    }
}

} // namespace pico
