#include "scorewidget.h"

#include <QApplication>
#include <QStyle>
#include <QVBoxLayout>

#include <QToolBar>
#include <QAction>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QScrollBar>
#include <QMessageBox>

#include <QDomElement>

ScoreWidget::ScoreWidget(const QDomElement &elem, QWidget *parent)
    : QWidget(parent),
      mToolBar(new QToolBar), mActPrevWrong(new QAction), mActNextWrong(new QAction),
      mLabelExam(new QLabel("考试名称: " + elem.attribute("ExamName"))), mLabelStu(new QLabel("学生名称: " + elem.attribute("StuName"))),
      mLabelScore(new QLabel("得分: " + elem.attribute("Score") + "/" + elem.attribute("TotalScore"))),
      mScrollArea(new QScrollArea), mScrollAreaWidget(new QWidget),
      mLayout(new QVBoxLayout), mLayoutWidgets(new QVBoxLayout)
{
    mActPrevWrong->setIcon(QIcon(":/icon/src/ArrowUp.png"));
    mActPrevWrong->setToolTip("前一个错题");
    mActNextWrong->setIcon(QIcon(":/icon/src/ArrowDown.png"));
    mActNextWrong->setToolTip("后一个错题");
    mLabelExam->setStyleSheet("color: white;");
    mLabelStu->setStyleSheet("color: white;");
    mLabelScore->setStyleSheet("color: white;");
    mToolBar->addAction(mActPrevWrong);
    mToolBar->addAction(mActNextWrong);
    mToolBar->addSeparator();
    mToolBar->addWidget(mLabelExam);
    mToolBar->addSeparator();
    mToolBar->addWidget(mLabelStu);
    mToolBar->addSeparator();
    mToolBar->addWidget(mLabelScore);
    mToolBar->setStyleSheet("QToolBar{background-color: rgb(24,204,192);}");
    mScrollArea->setObjectName("ScrArea");
    mScrollArea->setStyleSheet("QScrollArea#ScrArea{background-color: white;}"
                               "QLabel#LabelNum{"
                               "   border: 2px solid rgb(24, 204, 192);"
                               "   color: #FFFFFF;"
                               "   background-color: rgb(24, 204, 192);"
                               "}"
                               "QFrame#QuesBox{border: 1px solid #DDDDDD;}");
    mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mScrollArea->setWidgetResizable(true);
    mScrollAreaWidget->setObjectName("ScrAreaWidget");
    mScrollAreaWidget->setStyleSheet("QWidget#ScrAreaWidget{background-color: white;}");

    connect(mActPrevWrong, &QAction::triggered, this, &ScoreWidget::onPrevWrong);
    connect(mActNextWrong, &QAction::triggered, this, &ScoreWidget::onNextWrong);

    int i = 1;
    QDomNode node = elem.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        if(!elem.isNull()) {
            mVecIsRight << elem.attribute("Right").toInt();

            QLabel *label = new QLabel(QString::number(i) + '.');
            label->setObjectName("LabelNum");
            QLabel *labelHtml = new QLabel(elem.text());
            labelHtml->setWordWrap(true);
            QFrame *frame = new QFrame;
            frame->setObjectName("QuesBox");

            QHBoxLayout *layoutFrame = new QHBoxLayout;
            layoutFrame->addWidget(labelHtml);
            frame->setLayout(layoutFrame);
            QHBoxLayout *layout = new QHBoxLayout;
            layout->addWidget(label, 0, Qt::AlignTop);
            layout->addWidget(frame, 1);

            mLayoutWidgets->addLayout(layout);

            ++i;
        }
        node = node.nextSibling();
    }
    mLayout->addLayout(mLayoutWidgets);
    mLayout->addStretch();
    mScrollAreaWidget->setLayout(mLayout);
    mScrollArea->setWidget(mScrollAreaWidget);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setContentsMargins(QMargins());
    layoutMain->setSpacing(0);
    layoutMain->addWidget(mToolBar);
    layoutMain->addWidget(mScrollArea);
    setLayout(layoutMain);
    resize(1240, 760);
    setWindowTitle("作答情况");
}

int ScoreWidget::currentInd(bool ignoreTop) {
    int value = mScrollArea->verticalScrollBar()->value();
    int count = mLayoutWidgets->count();
    int ind = -1;
    for(int i = 0; i < count; ++i) {
        if(ignoreTop && mLayoutWidgets->itemAt(i)->geometry().y() == value) {
            ind = i - 1;
            break;
        }
        if(mLayoutWidgets->itemAt(i)->geometry().y() > value) {
            ind = i - 1;
            break;
        }
    }
    return qMax(0, ind);
}

void ScoreWidget::onPrevWrong() {
    int ind = currentInd(true);
    if(ind == -1)
        return;
    for(int i = ind; i >= 0; --i) {
        if(!mVecIsRight[i]) {
            mScrollArea->verticalScrollBar()->setValue(mLayoutWidgets->itemAt(i)->geometry().y());
            break;
        }
    }
}
void ScoreWidget::onNextWrong() {
    int ind = currentInd(false);
    if(ind == -1)
        return;
    int count = mLayoutWidgets->count();
    for(int i = ind + 1; i < count; ++i) {
        if(!mVecIsRight[i]) {
            mScrollArea->verticalScrollBar()->setValue(mLayoutWidgets->itemAt(i)->geometry().y());
            break;
        }
    }
}
