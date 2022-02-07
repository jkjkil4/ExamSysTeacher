#include "ques.h"

#include <QLabel>
#include <QFrame>
#include <QHBoxLayout>

Ques::Ques(QWidget *parent)
    : QWidget(parent),
      mLabelNum(new QLabel("0.")), mFrame(new QFrame), mLayoutMain(new QHBoxLayout)
{
    mLabelNum->setStyleSheet(
                "QLabel{"
                "   border: 2px solid rgb(24, 204, 192);"
                "   color: #FFFFFF;"
                "   background-color: rgb(24, 204, 192);"
                "}");
    mFrame->setFrameShape(QFrame::Shape::Box);
    mFrame->setObjectName("QuesBox");
    mFrame->setStyleSheet("QFrame#QuesBox{border: 1px solid #DDDDDD;}");
    mLayoutMain->addWidget(mLabelNum, 0, Qt::AlignTop);
    mLayoutMain->addWidget(mFrame, 1);
    setLayout(mLayoutMain);
}

void Ques::setNumber(int num) {
    mLabelNum->setText(QString::number(num) + '.');
}

void Ques::edit() {}
void Ques::writeXml(QXmlStreamWriter &) {}
void Ques::readXml(const QDomElement &) {}
