#include "queswhether.h"
#include "ui_queswhetherdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QXmlStreamWriter>
#include <QDomDocument>

QuesWhetherData::QuesWhetherData(QObject *parent)
    : QuesData(parent) {}

void QuesWhetherData::writeXml(QXmlStreamWriter &xml) const {
    xml.writeStartElement("QuesWhether");
    xml.writeAttribute("LTxt", mTextLeft);
    xml.writeAttribute("RTxt", mTextRight);
    xml.writeAttribute("State", QString::number(mState));
    xml.writeCharacters(mQuesText);
    xml.writeEndElement();
}
void QuesWhetherData::readXml(const QDomElement &elem) {
    mTextLeft = elem.attribute("LTxt");
    mTextRight = elem.attribute("RTxt");
    mState = (DoubleSlideButton::State)elem.attribute("State").toInt();
    mQuesText = elem.text();
}
void QuesWhetherData::writeXmlWithoutTrueAns(QXmlStreamWriter &xml) const {
    xml.writeStartElement("QuesWhether");
    xml.writeAttribute("LTxt", mTextLeft);
    xml.writeAttribute("RTxt", mTextRight);
    xml.writeCharacters(mQuesText);
    xml.writeEndElement();
}
void QuesWhetherData::writeXmlTrueAns(QXmlStreamWriter &xml) const {
    xml.writeTextElement("v", QString::number(mState));
}
bool QuesWhetherData::isRight(const QString &str) const {
    return str.toInt() == mState;
}


QuesWhether::QuesWhether(QWidget *parent)
    : Ques(parent),
    mLabelQues(new QLabel("（判断题）")),
    mLabelBtnLeft(new QLabel("错")), mLabelBtnRight(new QLabel("对")), mButton(new DoubleSlideButton),
    mLayoutButton(new QHBoxLayout), mLayout(new QVBoxLayout)
{
    mLabelQues->setWordWrap(true);

    mLayoutButton->addWidget(mLabelBtnLeft);
    mLayoutButton->addWidget(mButton);
    mLayoutButton->addWidget(mLabelBtnRight);
    mLayoutButton->addStretch();

    mLayout->addWidget(mLabelQues);
    mLayout->addSpacing(16);
    mLayout->addLayout(mLayoutButton);
    mFrame->setLayout(mLayout);

    connect(mButton, &DoubleSlideButton::stateChanged, this, &QuesWhether::onStateChanged);
}

bool QuesWhether::edit() {
    QDialog dialog(this);
    Ui::QuesWhetherDialog ui;
    ui.setupUi(&dialog);
    ui.editQues->setPlainText(mData.mQuesText);
    ui.editLeft->setText(mData.mTextLeft);
    ui.editRight->setText(mData.mTextRight);

    if(dialog.exec()) {
        // 题目文字
        mData.mQuesText = ui.editQues->toPlainText();

        // 按钮文字
        mData.mTextLeft = ui.editLeft->text();
        mData.mTextRight = ui.editRight->text();

        updateWidgetsByData();
        return true;
    }

    return false;
}

QString QuesWhether::isDone() {
    return mButton->state() == DoubleSlideButton::Mid ? "未选择选项" : QString();
}

void QuesWhether::writeXml(QXmlStreamWriter &xml) const {
    mData.writeXml(xml);
}

void QuesWhether::readXml(const QDomElement &elem) {
    mData.readXml(elem);
    updateWidgetsByData();
}

void QuesWhether::updateWidgetsByData() {
    mLabelQues->setText("（判断题）" + mData.mQuesText);
    mLabelBtnLeft->setText(mData.mTextLeft);
    mLabelBtnRight->setText(mData.mTextRight);
    mButton->setState((DoubleSlideButton::State)mData.mState);
}

void QuesWhether::onStateChanged() {
    mData.mState = mButton->state();
    emit changed();
}

