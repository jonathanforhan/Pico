#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QTextEdit>

#include "util/Util.hpp"

namespace pico {

class Editor;

/**
 * Abstract class for Text, HTML, Markdown etc buffer to base themselves off
 */
class Buffer : public QTextEdit
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr)
        : QTextEdit(parent),
          m_children({}),
          m_grid(nullptr)
    {}

signals:
    void
    modeChange(Mode mode);

private:
    QList<Buffer *> m_children;
    QGridLayout *m_grid;
};

} // namespace pico
