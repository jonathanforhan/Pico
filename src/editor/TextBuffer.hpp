#pragma once

#include "editor/Buffer.hpp"

namespace pico {

class TextBuffer : public Buffer
{
    Q_OBJECT

public:
    explicit TextBuffer(QWidget *parent = nullptr);

private:
};

} // namespace pico
