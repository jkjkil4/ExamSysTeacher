#include "examhistoryitemwidget.h"

#include <QLabel>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>

ExamHistoryItemWidget::ExamHistoryItemWidget(const QString &dirName, const QString &name,
                                             const QDateTime &dt, bool isEnd, QWidget *parent)
    : QWidget(parent), mDirName(dirName),
      mLabelName(new QLabel(name)), mLabelDateTime(new QLabel("创建时间: " + dt.toString("yyyy/M/d HH:mm:ss"))),
      mLabelState(new QLabel(isEnd ? "已结束" : "进行中")),
      mLayoutLeft(new QVBoxLayout), mLayoutMain(new QHBoxLayout)
{
    QPalette pal = palette();
    QFont ft = font();

    pal.setColor(QPalette::Text, QColor(15, 162, 138));
    mLabelName->setPalette(pal);
    ft.setPointSize(11);
    mLabelName->setFont(ft);

    pal.setColor(QPalette::Text, Qt::darkGray);
    mLabelDateTime->setPalette(pal);
    ft.setPointSize(9);
    mLabelDateTime->setFont(ft);

    pal.setColor(QPalette::Text, isEnd ? Qt::black : QColor(20, 170, 20));
    mLabelState->setPalette(pal);

    mLayoutLeft->addWidget(mLabelName, 0, Qt::AlignLeft);
    mLayoutLeft->addWidget(mLabelDateTime, 0, Qt::AlignLeft);

    mLayoutMain->addLayout(mLayoutLeft);
    mLayoutMain->addStretch();
    mLayoutMain->addWidget(mLabelState);

    setLayout(mLayoutMain);
}
