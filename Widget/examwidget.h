#pragma once

#include <QWidget>

#include <QFile>
#include <QDateTime>
#include <QMap>

namespace Ui {
class ExamWidget;
}

class QuesData;

/**
 * @brief   考试控制窗口
 */
class ExamWidget : public QWidget
{
    Q_OBJECT
public:
    enum State { BeforeProc, Proc, AfterProc };

    explicit ExamWidget(const QString &dirName, bool hasEnd = false, QWidget *parent = nullptr);

    bool isVaild() { return mIsVaild; }

    /**
     * @brief   根据时间关系更新状态
     */
    void updateState();

private:
    Ui::ExamWidget *ui;
    QString mDirName, mDirPath;
    bool mHasEnd;

    bool mIsVaild = false;

    QString mName;
    QDateTime mDateTimeStart, mDateTimeEnd;
    bool mScoreInClient;

    QMap<QString, const QMetaObject *> availableQues;
    QVector<QuesData*> mListQues;

    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
};
