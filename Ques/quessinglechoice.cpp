#include "quessinglechoice.h"
#include "ui_quessinglechoiceeditdialog.h"

#include <QLabel>

#include <QVBoxLayout>

QuesSingleChoice::QuesSingleChoice(QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel("（单选题）"))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabelQues);
    mFrame->setLayout(layout);
}

void QuesSingleChoice::edit() {
    QDialog dialog;
    Ui::QuesSingleChoiceEditDialog ui;
    ui.setupUi(&dialog);
    ui.editQues->setPlainText(text);
    dialog.exec();
}

void QuesSingleChoice::writeXml(QXmlStreamWriter &xml) {
    Q_UNUSED(xml)
    // TODO: ...
}
void QuesSingleChoice::readXml(const QDomElement &elem) {
    Q_UNUSED(elem)
    // TODO: ...
}
