#pragma once

#include <QStackedLayout>
#include <QWidget>

#include "editor/Buffer.hpp"
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

    const InputHandler *
    getInputHandler(void);

    void
    addBuffer(Buffer *buffer);

    const Buffer *
    getBuffer(qsizetype index);

    void
    removeBuffer(qsizetype index = -1);

    Mode
    getMode(void);

public slots:
    void
    changeMode(Mode mode);

private:
    InputHandler *m_inputHandler;
    QList<Buffer *> m_bufferList;
    QStackedLayout *m_bufferStack;
};

} // namespace pico
