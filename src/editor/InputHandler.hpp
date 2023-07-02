#pragma once

#include <QKeyCombination>
#include <QObject>
#include <functional>

#include "util/Util.hpp"

namespace pico {

/**
 * All access of InputHandler is done through the Editor
 * friend class
 */
class InputHandler final : public QObject
{
    Q_OBJECT

private:
    struct value;
    typedef qint64 key64_t;
    typedef std::unordered_map<key64_t, value> keymap_t;
    typedef std::function<void()> callback_t;
    typedef struct value {
        value() = delete;
        value(keymap_t *next)
            : callable(false),
              next(next)
        {}
        value(callback_t callback)
            : callable(true),
              callback(callback)
        {}
        ~value()
        {
            if (!this->callable)
                delete next;
        }

        /* don't call next if callable is true and visa verse */
        const bool callable; /* indicates if the entry is callback */
        union {
            keymap_t *const next;
            const callback_t callback;
        };
    } value_t;

private:
    explicit InputHandler(QObject *parent = nullptr);

private:
    void
    handleKeyPress(key64_t key);

    void
    handleKeyRelease(key64_t key);

    void
    addBinding(QList<QKeyCombination> keys, util::Mode mode, const callback_t &fn);

    /* catches keyboard input, passes input it to
     * handleKey* functions if the event is a keypress
     * or forwards appropriate response */
    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

    void
    setMode(util::Mode mode);

    void
    resetMapIndex(void);

private:
    /* Tracks state of shift, control and alt */
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modifiers;

    util::Mode m_mode; /* Track mode state */

    /* A map of infinite maps/functions used for key chords */
    keymap_t m_keyMap;
    /* tracks the key chord current state */
    keymap_t *m_keyMapIndex;

private:
    friend class Editor;
};

} // namespace pico
