#include "Editor.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QTextEdit>
#include <memory>

namespace pico {

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_bufferList({}),
      m_currentBufferIndex(-1),
      m_keyInputHandler(new KeyInputHandler(this))
{
    this->installEventFilter((KeyEventFilter *)m_keyInputHandler->getKeyEventFilter());

    m_keyInputHandler->addBinding(
        []() {
            qDebug() << "H";
        },
        Qt::Key_H);
    m_keyInputHandler->addBinding(
        []() {
            qDebug() << "<S-H>";
        },
        Qt::Key_H, KeyBind::Mod::Shift);
    m_keyInputHandler->addBinding(
        []() {
            qDebug() << "<S-C-H>";
        },
        Qt::Key_H, KeyBind::Mod::Shift | KeyBind::Mod::Control);
    m_keyInputHandler->addBinding(
        []() {
            qDebug() << "<S-C-M-H>";
        },
        Qt::Key_H, KeyBind::Mod::Shift | KeyBind::Mod::Control | KeyBind::Mod::Alt);

    QHBoxLayout *layout = new QHBoxLayout(this);
    Buffer *b = new Buffer(this);
    layout->addWidget(b);
}

Editor::~Editor()
{}

// Public Functions

qint32
Editor::getCurrentBufferIndex()
{
    return m_currentBufferIndex;
}

// Public Slots

} // namespace pico
