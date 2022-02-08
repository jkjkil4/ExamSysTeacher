#include "quessinglechoice.h"

#include <QRadioButton>

QuesSingleChoice::QuesSingleChoice(QWidget *parent) : QuesChoice("（单选题）", parent)
{

}

QAbstractButton* QuesSingleChoice::createBtn(const QString &str) {
    return new QRadioButton(str);
}
