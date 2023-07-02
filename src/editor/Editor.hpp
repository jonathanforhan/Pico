#pragma once

#include <QStackedLayout>
#include <QWidget>

#include "MainWindow.hpp"
#include "editor/Buffer.hpp"
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

    MainWindow *
    getMainWindow(void);

    void
    forwardEventFilter(QWidget *widget);

    inline void
    addBinding(QList<QKeyCombination> keys, util::Mode mode, const std::function<void()> &fn);

    util::Mode
    mode(void) const;

    inline void
    setMode(util::Mode mode);

    inline bool
    isShiftPressed(void);

    inline bool
    isControlPressed(void);

    inline bool
    isAltPressed(void);

    inline void
    nextBuffer(void);

    inline void
    prevBuffer(void);

    inline void
    nthBuffer(qsizetype i);

    inline Buffer *
    getCurrentBuffer(void);

    inline void
    addBuffer(Buffer *buffer);

    inline void
    removeBuffer(Buffer *buffer);

private:
    InputHandler *m_inputHandler;
    QStackedLayout *m_bufferStack;

private: /* Singleton implementation */
    Q_INVOKABLE
    Editor(QWidget *parent = nullptr);
    /* Init must be done after constructor
     * to ensure no data races */
    void
    Init(void);

    Editor(const Editor &) = delete;

    void
    operator=(const Editor &) = delete;
};

} // namespace pico
