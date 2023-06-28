#include "Editor.hpp"

#include "editor/TextBuffer.hpp"
#include "util/Util.hpp"

namespace pico {

Editor *Editor::s_instance = nullptr;

Editor::Editor(QWidget *parent)
    : QWidget(parent),
      m_inputHandler(new InputHandler(this)),
      m_bufferList({}),
      m_bufferStack(new QStackedLayout)
{
    setLayout(m_bufferStack);
    m_bufferStack->setSpacing(0);
    m_bufferStack->setContentsMargins(0, 0, 0, 0);

    TextBuffer *entryBuffer = new TextBuffer(this);
    addBuffer(entryBuffer);
    entryBuffer->setText("Welcome to Pico Editor");

    m_inputHandler->addBinding({ Key::H | ModKey::Control }, Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == 0 ? m_bufferStack->count() - 1 : i - 1;
        m_bufferStack->setCurrentIndex(i);
        m_bufferList[i]->setFocus();
    });

    m_inputHandler->addBinding({ Key::L | ModKey::Control }, Mode::Normal, [=]() {
        int i = m_bufferStack->currentIndex();
        i = i == m_bufferStack->count() - 1 ? 0 : i + 1;
        m_bufferStack->setCurrentIndex(i);
        m_bufferList[i]->setFocus();
    });

    m_inputHandler->addBinding({ Key::Space, Key::B, Key::N }, Mode::Normal, [=]() {
        qDebug() << m_bufferList.count();
        addBuffer(new TextBuffer(this));
    });

    m_inputHandler->addBinding({ Key::Space, Key::B, Key::D }, Mode::Normal, [=]() {
        removeBuffer();
    });

    m_inputHandler->addBinding({ Key::I }, Mode::Normal, [=]() {
        m_inputHandler->setMode(Mode::Insert);
    });

    m_inputHandler->addBinding({ Qt::Key_Escape }, Mode::Insert, [=]() {
        m_inputHandler->setMode(Mode::Normal);
    });
}

Editor *
Editor::getInstance(QWidget *parent)
{
    if (s_instance == nullptr)
        s_instance = new Editor(parent);
    return s_instance;
}

const InputHandler *
Editor::getInputHandler(void)
{
    return m_inputHandler;
}

void
Editor::addBuffer(Buffer *buffer)
{
    buffer->installEventFilter((InputHandler *)this->getInputHandler());
    connect(buffer, &Buffer::modeChange, this, &Editor::changeMode);

    m_bufferList.append(buffer);
    m_bufferStack->addWidget(buffer);
}

const Buffer *
Editor::getBuffer(qsizetype index)
{
    Q_ASSERT(index < m_bufferList.size());
    return m_bufferList.at(index);
}

void
Editor::removeBuffer(qsizetype index)
{
    if (index == -1)
        index = m_bufferStack->currentIndex();
    Q_ASSERT(index < m_bufferList.size());
    m_bufferStack->removeWidget(m_bufferList.at(index));
    m_bufferList.remove(index);
}

Mode
Editor::getMode(void)
{
    return m_inputHandler->getMode();
}

/* slots */

void
Editor::changeMode(Mode mode)
{
    qDebug() << "Mode Changed";
    m_inputHandler->setMode(mode);
}

} // namespace pico
