#pragma once

#include "ques.h"

class QVBoxLayout;
class QAbstractButton;

class QuesChoiceData : public QuesData
{
    Q_OBJECT
public:
    struct Choice { bool isChecked; QString text; };

    Q_INVOKABLE explicit QuesChoiceData(const QString &quesName, const QString &head, QObject *parent = nullptr);

    QString ansStr() const;

    void writeXml(QXmlStreamWriter &xml) const override;
    void readXml(const QDomElement &elem) override;
    void writeXmlWithoutTrueAns(QXmlStreamWriter &xml) const override;
    void writeXmlTrueAns(QXmlStreamWriter &xml) const override;
    Score score(const QString &str) const override;

    QString quesName;
    QString head;
    QString quesText;
    QList<Choice> choiceList;
};

/**
 * @brief   选项题
 */
class QuesChoice : public Ques
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesChoice(const QString &quesName, const QString &head, QWidget *parent = nullptr);

    bool edit() override;

    void writeXml(QXmlStreamWriter &xml) const override;
    void readXml(const QDomElement &elem) override;

    void updateWidgetsByData();

    /**
     * @brief   用于让子类自定Btn类
     * @param   Btn文字
     * @return  自定的Btn类的对象
     */
    virtual QAbstractButton* createBtn(const QString &str);

protected:
    QLabel *mLabelQues;
    QVBoxLayout *mLayout, *mLayoutButtons;

    QString mHead;
    QuesChoiceData mData;
};
