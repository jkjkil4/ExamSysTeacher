#pragma once

#include "ques.h"

class QRadioButton;
class QVBoxLayout;

/**
 * @brief   单选题
 */
class QuesSingleChoice : public Ques
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesSingleChoice(QWidget *parent = nullptr);

    Ques* edit() const override;
    void writeXml(QXmlStreamWriter &xml) const override;
    void readXml(const QDomElement &elem) override;

private:
    QLabel *mLabelQues;
//    QList<QRadioButton*> mRadioButtonList;
    QVBoxLayout *mLayout;

    QString text;
    QStringList list;
    int trueAns = -1;
};
