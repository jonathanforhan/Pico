#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QTextEdit>

namespace pico {

class Buffer : public QTextEdit
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

    void
    keyPressEvent(QKeyEvent *event) override;

private:
    QList<Buffer *> m_children;
    QGridLayout *m_grid;
};

} // namespace pico
