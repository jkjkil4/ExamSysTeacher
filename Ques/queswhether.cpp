#include "queswhether.h"
#include "ui_queswhetherdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QXmlStreamWriter>
#include <QDomDocument>

#include "Widget/doubleslidebutton.h"

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

    connect(mButton, SIGNAL(stateChanged()), this, SIGNAL(changed()));
}

bool QuesWhether::edit() {
    QDialog dialog(this);
    Ui::QuesWhetherDialog ui;
    ui.setupUi(&dialog);
    ui.editQues->setPlainText(mText);
    ui.editLeft->setText(mLabelBtnLeft->text());
    ui.editRight->setText(mLabelBtnRight->text());

    if(dialog.exec()) {
        mText = ui.editQues->toPlainText();
        mLabelQues->setText("（判断题）" + mText);
        mLabelBtnLeft->setText(ui.editLeft->text());
        mLabelBtnRight->setText(ui.editRight->text());

        return true;
    }

    return false;
}

QString QuesWhether::isDone() {
    return mButton->state() == DoubleSlideButton::Mid ? "未选择选项" : QString();
}

void QuesWhether::writeXml(QXmlStreamWriter &xml) const {
    xml.writeStartElement("QuesWhether");
    xml.writeAttribute("State", QString::number(mButton->state()));
    xml.writeCharacters(mText);
    xml.writeEndElement();
}

void QuesWhether::readXml(const QDomElement &elem) {
    mButton->blockSignals(true);
    mButton->setState((DoubleSlideButton::State)elem.attribute("State").toInt());
    mButton->blockSignals(false);
    mText = elem.text();
    mLabelQues->setText("（判断题）" + mText);
}
