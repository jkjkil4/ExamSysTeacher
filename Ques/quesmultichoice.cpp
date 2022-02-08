#include "quesmultichoice.h"

#include <QCheckBox>

QuesMultiChoice::QuesMultiChoice(QWidget *parent) : QuesChoice("（多选题）", parent)
{

}

QAbstractButton* QuesMultiChoice::createBtn(const QString &str) {
    return new QCheckBox(str);
}
