#pragma once

#include <QObject>
#include <any>
#include <functional>

#include "util/Util.hpp"

namespace pico {

namespace binding {

typedef int key_t; /* key | modifiers */
typedef struct {
    std::shared_ptr<void> link; /* link to next map or function */
    bool callable;              /* indicates if the entry is function */
} entry_t;
typedef std::unordered_map<key_t, entry_t> keymap_t;
typedef std::function<void()> callback_t;

} // namespace binding

class InputHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputHandler(QObject *parent = nullptr);

    void
    handleKeyPress(key_t key);

    void
    handleKeyRelease(key_t key);

    void
    addBinding(QList<key_t> keys, Mode mode, const binding::callback_t &fn);

    /* catches keyboard input, passes input it to
     * handleKey* functions if the event is a keypress
     * or forwards appropriate response */
    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

    void
    setMode(Mode mode);

    Mode
    getMode(void);

private:
    void
    resetMapIndex(void);

    /* Tracks state of shift, control and alt */
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modifiers;

    Mode m_mode; /* Track mode state */

    /* A map of infinite maps/functions used for key chords */
    binding::keymap_t m_keyMap;
    /* tracks the key chord current state */
    binding::keymap_t *m_keyMapIndex;
};

} // namespace pico
