#pragma once

#include "queschoice.h"

class QuesMultiChoice : public QuesChoice
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesMultiChoice(QWidget *parent = nullptr);

    QString isDone() override;
    QAbstractButton* createBtn(const QString &str) override;
};
