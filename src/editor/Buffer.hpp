#pragma once

#include <QGridLayout>
#include <QTextEdit>

namespace pico {

/**
 * Abstract class for Text, HTML, Markdown etc buffer to base themselves off
 */
class Buffer : public QTextEdit
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr)
        : QTextEdit(parent)
    {}

protected:
    // TODO functions accessing the children

private:
    QList<Buffer *> children;
    QGridLayout *grid;
};

} // namespace pico
