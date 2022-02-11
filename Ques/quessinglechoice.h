#pragma once

#include "queschoice.h"

class QuesSingleChoice : public QuesChoice
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesSingleChoice(QWidget *parent = nullptr);

    QString isDone() override;
    QAbstractButton* createBtn(const QString &str) override;
};
