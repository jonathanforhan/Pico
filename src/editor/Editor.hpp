#pragma once

#include <QWidget>

#include "editor/BufferStack.hpp"
#include "editor/InputHandler.hpp"

namespace pico {

/**
 * Editor is the MainWindow's central widget,
 * Editor is a singleton that in gotten through
 * it's instance pointer
 */
class Editor : public QWidget
{
    Q_OBJECT

protected:
    explicit Editor(QWidget *parent = nullptr);
    static Editor *s_instance;

public:
    Editor(const Editor &) = delete;
    void
    operator=(const Editor &) = delete;

    static Editor *
    getInstance(QWidget *parent = nullptr);

    InputHandler *
    getInputHandler(void);

    Mode
    getMode(void);

    void
    setMode(Mode mode);

private:
    InputHandler *m_inputHandler;
    BufferStack *m_bufferStack;
};

} // namespace pico
