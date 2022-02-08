#pragma once

#include "queschoice.h"

class QuesMultiChoice : public QuesChoice
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesMultiChoice(QWidget *parent = nullptr);

    QAbstractButton* createBtn(const QString &str) override;
};
