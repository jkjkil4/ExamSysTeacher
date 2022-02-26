#pragma once

#include <QDialog>

namespace Ui {
class ExamHistory;
}

class ExamHistory : public QDialog
{
    Q_OBJECT
public:
    explicit ExamHistory(QWidget *parent = nullptr);

    QString examDirName() { return mExamDirName; }

private:
    Ui::ExamHistory *ui;

    QString mExamDirName;
};
