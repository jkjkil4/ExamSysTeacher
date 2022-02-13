#pragma once

#include <QWidget>

#include <QFile>
#include <QDateTime>

namespace Ui {
class ExamWidget;
}

class ExamWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExamWidget(const QString &dirName, QWidget *parent = nullptr);

    bool isVaild() { return mIsVaild; }

private:
    Ui::ExamWidget *ui;
    QString mDirName, mDirPath;

    bool mIsVaild = false;

    QString mName;
    QDateTime mDateTimeStart, mDateTimeEnd;
    bool mScoreInClient;
    int mQuesCnt;

    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
};
