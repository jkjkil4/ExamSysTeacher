#pragma once

#include "queschoice.h"

class QuesMultiChoiceData : public QuesChoiceData
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesMultiChoiceData(QObject *parent = nullptr);
};

class QuesMultiChoice : public QuesChoice
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesMultiChoice(QWidget *parent = nullptr);

    QString isDone() override;
    QAbstractButton* createBtn(const QString &str) override;
};
