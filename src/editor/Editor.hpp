#pragma once

#include <QWidget>

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
    getInstance(QWidget *w = nullptr);

    const InputHandler *
    getInputHandler();

private:
    InputHandler *m_inputHandler;
};

} // namespace pico
