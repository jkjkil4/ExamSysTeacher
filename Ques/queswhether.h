#pragma once

#include "ques.h"
#include "Widget/doubleslidebutton.h"

class QVBoxLayout;

class QuesWhetherData : public QuesData
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesWhetherData(QObject *parent = nullptr);

    void writeXml(QXmlStreamWriter &xml) const override;
    void readXml(const QDomElement &elem) override;
    void writeXmlWithoutTrueAns(QXmlStreamWriter &xml, int ind) const override;

    QString mQuesText;
    QString mTextLeft, mTextRight;
    DoubleSlideButton::State mState;
};

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

    void updateWidgetsByData();

private:
    QLabel *mLabelQues;
    QLabel *mLabelBtnLeft, *mLabelBtnRight;
    DoubleSlideButton *mButton;
    QHBoxLayout *mLayoutButton;
    QVBoxLayout *mLayout;

    QuesWhetherData mData;
};
