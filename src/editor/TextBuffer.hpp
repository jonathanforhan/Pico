#pragma once

#include <QKeyEvent>

#include "editor/Buffer.hpp"

namespace pico {

class TextBuffer : public Buffer
{
    Q_OBJECT

public:
    explicit TextBuffer(QWidget *parent = nullptr);

    void
    keyPressEvent(QKeyEvent *event) override;

private:
};

} // namespace pico
