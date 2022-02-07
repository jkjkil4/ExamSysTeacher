#pragma once

#include "ques.h"

/**
 * @brief   单选题
 */
class QuesSingleChoice : public Ques
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesSingleChoice(QWidget *parent = nullptr);

    void edit() override;
    void writeXml(QXmlStreamWriter &xml) override;
    void readXml(const QDomElement &elem) override;

private:
    QLabel *mLabelQues;
    QString text;
};
