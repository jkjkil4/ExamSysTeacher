#include "pushview.h"
#include "ui_pushview.h"

#include <QDateTime>

PushView::PushView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PushView)
{
    ui->setupUi(this);

    connect(ui->btnBack, SIGNAL(clicked()), this, SIGNAL(back()));
}

PushView::~PushView()
{
    delete ui;
}

void PushView::autoAdjustDateTime() {
    QDateTime dateTime = QDateTime::currentDateTime();
    QTime time = dateTime.time();
    time.setHMS(time.hour(), 0, 0);
    dateTime.setTime(time);
    ui->dateTimeEditStart->setDateTime(dateTime.addSecs(60 * 60));
    ui->dateTimeEditEnd->setDateTime(dateTime.addSecs(2 * 60 * 60));
}
