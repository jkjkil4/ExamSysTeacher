#include "examwidget.h"
#include "ui_examwidget.h"

ExamWidget::ExamWidget(const QString &dirName, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExamWidget)
{
    ui->setupUi(this);
}
