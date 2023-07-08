#pragma once

#include "editor/KeyListener.hpp"
#include <QObject>

namespace pico {

class KeyFilter : public KeyListener
{
    Q_OBJECT

public:
    explicit KeyFilter(QObject *parent = nullptr);

private:
    bool
    handleKeyRelease(key64_t key);

    bool
    handleKeyPress(key64_t key) override;

    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;
};

} // namespace pico
