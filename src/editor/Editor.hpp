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
class Editor final : public QWidget
{
    Q_OBJECT

public:
    static Editor *
    getInstance(QWidget *parent = nullptr);

    InputHandler *
    getInputHandler(void);

    Mode
    getMode(void) const;

    void
    setMode(Mode mode);

    const QFont &
    getFont(void) const;

    void
    setFont(QFont &font);

private:
    InputHandler *m_inputHandler;
    BufferStack *m_bufferStack;

    /* Editor Settings */
    QFont m_font;

    /* Singleton implementation */
private:
    explicit Editor(QWidget *parent = nullptr);

    Editor(const Editor &) = delete;
    void
    operator=(const Editor &) = delete;
    /* Init must be done after constructor in the MainWindow so
     * the instance is created (construct depends on ::getInstance) */
    void
    Init(void);
    friend class MainWindow;
};

} // namespace pico
