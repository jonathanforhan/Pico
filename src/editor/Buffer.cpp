#include "Buffer.hpp"
#include "editor/FileTree.hpp"

#include <QApplication>

namespace pico {

Buffer::Buffer(QWidget *parent)
    : QWidget(parent),
      PicoObject(this),
      m_layout(new QHBoxLayout(this)),
      m_splitter(new QSplitter(this)),
      m_fileTree(new FileTree(m_splitter))
{
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_splitter);
    m_splitter->setHandleWidth(0);

    m_splitter->addWidget(new TextEdit(this));
}

QSplitter *
Buffer::splitter()
{
    return m_splitter;
}

void
Buffer::splitLeft(QWidget *widget)
{
    m_layout->removeWidget(m_splitter);
    auto *oldSplitter = m_splitter;
    m_splitter = new QSplitter(this);
    m_splitter->setHandleWidth(0);
    m_splitter->addWidget(widget);
    m_splitter->addWidget(oldSplitter);
    m_layout->addWidget(m_splitter);
    widget->setFocus();
}

void
Buffer::splitRight(QWidget *widget)
{
    m_layout->removeWidget(m_splitter);
    auto *oldSplitter = m_splitter;
    m_splitter = new QSplitter(this);
    m_splitter->setHandleWidth(0);
    m_splitter->addWidget(oldSplitter);
    m_splitter->addWidget(widget);
    m_layout->addWidget(m_splitter);
    widget->setFocus();
}

void
Buffer::splitTop(QWidget *widget)
{
    m_layout->removeWidget(m_splitter);
    auto *oldSplitter = m_splitter;
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setHandleWidth(0);
    m_splitter->addWidget(widget);
    m_splitter->addWidget(oldSplitter);
    m_layout->addWidget(m_splitter);
    widget->setFocus();
}

void
Buffer::splitBottom(QWidget *widget)
{
    m_layout->removeWidget(m_splitter);
    auto *oldSplitter = m_splitter;
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setHandleWidth(0);
    m_splitter->addWidget(oldSplitter);
    m_splitter->addWidget(widget);
    m_layout->addWidget(m_splitter);
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

} // namespace pico
