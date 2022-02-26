#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;

class ExamHistoryItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExamHistoryItemWidget(const QString &dirName, const QString &name,
                                   const QDateTime &dt, bool isEnd, QWidget *parent = nullptr);

    QString dirName() const { return mDirName; }

private:
    QString mDirName;

    QLabel *mLabelName, *mLabelDateTime, *mLabelState;
    QVBoxLayout *mLayoutLeft;
    QHBoxLayout *mLayoutMain;
};

