#pragma once

#include <QKeyEvent>

#include "editor/Buffer.hpp"

namespace pico {

class TextBuffer : public Buffer
{
    Q_OBJECT

public:
    explicit TextBuffer(Editor *parent = nullptr);

    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

private:
    Editor *m_editor;
};

} // namespace pico
