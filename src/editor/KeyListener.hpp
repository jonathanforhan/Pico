#pragma once

#include <QObject>
#include <functional>
#include <unordered_map>

#include "editor/KeyListenerPrivate.hpp"
#include "util/Util.hpp"

namespace pico {

class KeyListener : public QObject, private KeyListenerPrivate
{
    Q_OBJECT

public:
    explicit KeyListener(QObject *parent = nullptr);

    void
    handleKeyPress(key64_t key);

    bool
    addBinding(QList<QKeyCombination> keyCombo, Mode mode, callback_t callback);

private:
    void
    resetMapIndex(void);

private:
    /* A map of infinite maps/functions used for key chords */
    keymap_t m_keyMap;
    /* tracks the key chord current state */
    keymap_t *m_keyMapIndex;
};

} // namespace pico
