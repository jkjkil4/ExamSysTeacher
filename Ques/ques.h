#pragma once

#include <QWidget>

class QLabel;
class QFrame;
class QHBoxLayout;

class QXmlStreamWriter;
class QDomElement;

/**
 * @brief 题目基类
 * 定义了 writeXml readXml 等虚函数和其他函数
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
     * @brief   使用XML方式对题目内容进行保存
     * @param   xml     QXmlStreamWriter对象，用于写入XML
     */
    virtual void writeXml(QXmlStreamWriter &xml) const;
    /**
     * @brief   使用XML方式对题目内容进行读取
     * @param   elem    QDomElement对象，用于读取XML
     */
    virtual void readXml(const QDomElement &elem);

protected:
    void mouseDoubleClickEvent(QMouseEvent *ev) override;

protected:
    QLabel *mLabelNum;
    QFrame *mFrame;
    QHBoxLayout *mLayoutMain;
};
