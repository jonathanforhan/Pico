#pragma once

#include <QList>
#include <QStackedLayout>
#include <QWidget>
#include <memory>

#include "editor/Buffer.hpp"
#include "editor/InputHandler.hpp"

namespace pico {

class BufferStack : public QStackedLayout
{
    Q_OBJECT

public:
    explicit BufferStack(Editor *parent);

    void
    addWidget(Buffer *buffer);

private:
    InputHandler *m_inputHandler;
};

} // namespace pico
