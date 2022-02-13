#pragma once

#include <QWidget>

namespace Ui {
class ExamWidget;
}

class ExamWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExamWidget(const QString &dirName, QWidget *parent = nullptr);

private:
    Ui::ExamWidget *ui;
};
