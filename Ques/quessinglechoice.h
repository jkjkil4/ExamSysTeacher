#pragma once

#include "queschoice.h"

class QRadioButton;

class QuesSingleChoice : public QuesChoice
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesSingleChoice(QWidget *parent = nullptr);

    QAbstractButton* createBtn(const QString &str) override;
};
