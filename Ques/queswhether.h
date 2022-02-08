#pragma once

#include "ques.h"

class QVBoxLayout;

class QuesWhether : public Ques
{
public:
    Q_INVOKABLE explicit QuesWhether(QWidget *parent = nullptr);

private:
    QLabel *mLabelQues;
    QVBoxLayout *mLayout;

    QString mText;
};
