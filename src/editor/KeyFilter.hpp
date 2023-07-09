#pragma once

#include <QObject>

#include "editor/KeyListener.hpp"

namespace pico {

class KeyFilter : public KeyListener
{
    Q_OBJECT

public:
    explicit KeyFilter(QObject *parent = nullptr);

    bool
    handleKeyRelease(key64_t key);

    bool
    handleKeyPress(key64_t key) override;

private:
    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

private:
    void *m_editor;
};

} // namespace pico
