#include "pushview.h"
#include "ui_pushview.h"

PushView::PushView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PushView)
{
    ui->setupUi(this);
}

PushView::~PushView()
{
    delete ui;
}
