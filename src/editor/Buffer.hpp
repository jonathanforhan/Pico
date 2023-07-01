#pragma once

#include <QGridLayout>
#include <QTextEdit>

namespace pico {

class Buffer : public QTextEdit
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

private:
    QList<Buffer *> m_children;
    QGridLayout *m_grid;
};

} // namespace pico
