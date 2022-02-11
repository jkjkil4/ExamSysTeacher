#include "quessinglechoice.h"

#include <QRadioButton>
#include <QVBoxLayout>

QuesSingleChoice::QuesSingleChoice(QWidget *parent) : QuesChoice("（单选题）", parent)
{

}

QString QuesSingleChoice::isDone() {
    int count = mLayoutButtons->count();
    for(int i = 0; i < count; ++i) {
        QAbstractButton *btn = (QAbstractButton*)mLayoutButtons->itemAt(i)->widget();
        if(btn->isChecked())
            return QString();
    }
    return "未选择选项";
}

QAbstractButton* QuesSingleChoice::createBtn(const QString &str) {
    return new QRadioButton(str);
}
