#include "filter_base.h"
#include "ui_filter_base.h"

Filter_base::Filter_base(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Filter_base)
{
    ui->setupUi(this);
}

Filter_base::~Filter_base()
{
    delete ui;
}
