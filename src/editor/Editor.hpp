#pragma once

#include <QKeyEvent>
#include <QWidget>

#include "editor/Buffer.hpp"
#include "editor/KeyInputHandler.hpp"

namespace pico {

class Editor : public QWidget
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = nullptr);
    ~Editor() override;

    qint32
    getCurrentBufferIndex();

private:
    QList<Buffer *> m_bufferList;
    qint32 m_currentBufferIndex;
    KeyInputHandler *m_keyInputHandler;
};

} // namespace pico
