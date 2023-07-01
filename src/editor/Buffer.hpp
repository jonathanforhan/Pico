#pragma once

#include <QGridLayout>
#include <QTextEdit>

namespace pico {

class Buffer : public QTextEdit
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

private:
    QList<Buffer *> m_children;
    QGridLayout *m_grid;
};

} // namespace pico
