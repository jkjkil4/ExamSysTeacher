#pragma once

#include "ques.h"

class QVBoxLayout;
class DoubleSlideButton;

/**
 * @brief   判断题
 */
class QuesWhether : public Ques
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesWhether(QWidget *parent = nullptr);

    bool edit() override;
    QString isDone() override;
    void writeXml(QXmlStreamWriter &xml) const override;
    void readXml(const QDomElement &elem) override;

private:
    QLabel *mLabelQues;
    QLabel *mLabelBtnLeft, *mLabelBtnRight;
    DoubleSlideButton *mButton;
    QHBoxLayout *mLayoutButton;
    QVBoxLayout *mLayout;

    QString mText;
};
