#pragma once

#include <QWidget>

class QLabel;
class QFrame;
class QHBoxLayout;

class QXmlStreamWriter;
class QDomElement;

/**
 * @brief 题目数据基类
 * 定义了 writeXml readXml 等虚函数和其他函数
 */
class QuesData : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QuesData(QObject *parent = nullptr);

    /**
     * @brief   使用XML方式对题目内容进行保存
     * @param   xml     QXmlStreamWriter对象，用于写入XML
     */
    virtual void writeXml(QXmlStreamWriter &xml) const;
    /**
     * @brief   使用XML方式对题目内容进行读取
     * @param   elem    QDomElement对象，用于读取XML
     */
    virtual void readXml(const QDomElement &elem);

    /**
     * @brief   使用XML方式对题目内容进行写入，不含答案，用于传给客户端
     * @param   xml     QXmlStreamWriter对象，用于写入XML
     * @param   ind     题目的索引
     */
    virtual void writeXmlWithoutTrueAns(QXmlStreamWriter &xml, int ind) const;
};

/**
 * @brief 题目控件基类
 */
class Ques : public QWidget
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit Ques(QWidget *parent = nullptr);

    /**
     * @brief   设置序号
     * @param   num     序号
     */
    void setNumber(int num);

    /**
     * @brief   响应编辑操作
     */
    virtual bool edit();

    /**
     * @brief   返回是否完成编辑
     * @return  返回的字符串
     * 若返回的为空字符串，则完成
     * 若返回的不为空字符串，则未完成，并且字符串表示相关提示
     */
    virtual QString isDone();

    virtual void writeXml(QXmlStreamWriter &xml) const;
    virtual void readXml(const QDomElement &elem);

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev) override;

signals:
    void changed();
    void doubleClicked();

protected:
    QLabel *mLabelNum;
    QFrame *mFrame;
    QHBoxLayout *mLayoutMain;
};
