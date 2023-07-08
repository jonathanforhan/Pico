#pragma once

#include <QObject>

#include "editor/KeyListenerTypes.hpp"
#include "util/Util.hpp"

namespace pico {

class KeyListener : public QObject, public KeyListenerTypes
{
    Q_OBJECT

public:
    explicit KeyListener(QObject *parent = nullptr);

    virtual bool
    handleKeyPress(key64_t key);

    bool
    addBinding(QList<QKeyCombination> keyCombo, Mode mode, callback_t callback);

protected:
    void
    resetMapIndex(void);

protected:
    /* A map of infinite maps/functions used for key chords */
    keymap_t m_keyMap;
    /* tracks the key chord current state */
    keymap_t *m_keyMapIndex;
};

} // namespace pico
