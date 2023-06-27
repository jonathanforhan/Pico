#pragma once

#include <QTabWidget>

namespace pico {

class Buffer : public QTabWidget
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

private:
    QLayout *m_layout;
};

} // namespace pico
