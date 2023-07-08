#pragma once

#include <QObject>

namespace pico {

class KeyListenerTypes
{
protected: /* typedecl */
    /* Type declarations */
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
};

} // namespace pico
