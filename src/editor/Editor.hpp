#pragma once

#include <QKeyEvent>
#include <QMainWindow>
#include <QStackedLayout>
#include <QWidget>

#include "editor/KeyFilter.hpp"
#include "util/Util.hpp"

namespace pico {

/**
 * Singleton used to get and set editor state
 */
class Editor final : public QWidget
{
    Q_OBJECT

public: /* functions */
    static Editor *
    getInstance(QMainWindow *parent = nullptr);

    void
    forwardKeyFilter(QObject *obj);

    bool
    shiftState(void);

    bool
    controlState(void);

    bool
    altState(void);

    Mode
    mode(void);

    void
    setMode(Mode mode);

    void
    addBuffer(void);

    void
    nextBuffer(void);

    void
    prevBuffer(void);

    void
    nthBuffer(int index);

private: /* vars */
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modifiers;
    Mode m_mode;
    KeyFilter m_keyFilter;
    QStackedLayout *m_stack;

private: /* singleton setup */
    explicit Editor(QMainWindow *parent = nullptr);

    void
    Init(void);

    Editor(const Editor &) = delete;

    void
    operator=(const Editor &) = delete;

private:
    friend class KeyFilter;
};

} // namespace pico
