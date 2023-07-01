#pragma once

#include <QStackedLayout>
#include <QWidget>

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
    getInputHandler(void) const;

    util::Mode
    getMode(void) const;

    void
    setMode(util::Mode mode);

private:
    InputHandler *m_inputHandler;
    QStackedLayout *m_bufferStack;

private: /* Singleton implementation */
    explicit Editor(QWidget *parent = nullptr);
    /* Init must be done after constructor
     * to ensure no data races */
    void
    Init(void);

    Editor(const Editor &) = delete;

    void
    operator=(const Editor &) = delete;
};

} // namespace pico
