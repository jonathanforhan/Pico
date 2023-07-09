#pragma once

#include <QGridLayout>
#include <QWidget>

#include <editor/TextEdit.hpp>

namespace pico {

class Buffer : public QWidget, public PicoObject
{
    Q_OBJECT

public:
    explicit Buffer(QWidget *parent = nullptr);

private:
    QGridLayout *m_layout;
};

} // namespace pico
