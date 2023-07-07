#pragma once

#include "util/Util.hpp"
#include <QKeyEvent>
#include <QMainWindow>
#include <QWidget>

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

private: /* vars */
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modState;

    Mode m_mode;

private: /* singleton setup */
    explicit Editor(QMainWindow *parent = nullptr);

    constexpr void
    Init(void);

    Editor(const Editor &) = delete;

    void
    operator=(const Editor &) = delete;
};

} // namespace pico
