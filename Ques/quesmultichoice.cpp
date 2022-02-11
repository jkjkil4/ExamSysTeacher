#include "quesmultichoice.h"

#include <QCheckBox>
#include <QVBoxLayout>

QuesMultiChoice::QuesMultiChoice(QWidget *parent) : QuesChoice("（多选题）", parent)
{

}

QString QuesMultiChoice::isDone() {
    int count = mLayoutButtons->count();
    int checkedCnt = 0;
    for(int i = 0; i < count; ++i) {
        QAbstractButton *btn = (QAbstractButton*)mLayoutButtons->itemAt(i)->widget();
        if(btn->isChecked())
            ++checkedCnt;
    }
    if(checkedCnt == 0)
        return "未选择选项";
    if(checkedCnt == 1)
        return "选项应不少于2个";
    return QString();
}

QAbstractButton* QuesMultiChoice::createBtn(const QString &str) {
    return new QCheckBox(str);
}
