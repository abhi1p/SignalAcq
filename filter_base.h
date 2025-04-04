#ifndef FILTER_BASE_H
#define FILTER_BASE_H

#include <QWidget>

namespace Ui {
class Filter_base;
}

class Filter_base : public QWidget
{
    Q_OBJECT

public:
    explicit Filter_base(QWidget *parent = nullptr);
    ~Filter_base();

private:
    Ui::Filter_base *ui;
};

#endif // FILTER_BASE_H
