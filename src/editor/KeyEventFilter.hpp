#pragma once

#include <QKeyEvent>
#include <QObject>

namespace pico {

class KeyEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KeyEventFilter(QObject *parent = nullptr);

signals:
    void
    keyPress(Qt::Key key);

    void
    keyRelease(Qt::Key key);

protected:
    [[nodiscard]] bool
    eventFilter(QObject *obj, QEvent *event) override;

private:
    QKeyEvent *m_key;
};

} // namespace pico
