#pragma once

#include <QObject>
#include <any>
#include <functional>

namespace pico {

namespace binding {

enum Mod {
    None = 0x0000,
    Shift = 0x0100,
    Control = 0x0200,
    Alt = 0x0400,
};

typedef int key_t; // key + modifiers
typedef std::pair<std::shared_ptr<void>, bool> child_t;
typedef std::unordered_map<key_t, child_t> keymap_t;
typedef std::function<void()> callback_t;

} // namespace binding

class InputHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputHandler(QObject *parent = nullptr);

    void
    handleKeyPress(Qt::Key key);

    void
    handleKeyRelease(Qt::Key key);

    void
    addBinding(QList<key_t> keys, const binding::callback_t &fn);

    /* catches keyboard input, passes input it to
     * handleKey* functions if the event is a keypress
     * or forwards appropriate response */
    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

private:
    /* Tracks state of shift, control and alt */
    struct {
        unsigned shift : 2;
        unsigned control : 2;
        unsigned alt : 2;
    } m_modifiers;

    /* A map of infinite maps/functions used for key chords, beware */
    binding::keymap_t m_keyMap;
    /* tracks the key chord current state */
    binding::keymap_t *m_keyMapIndex;
};

} // namespace pico
