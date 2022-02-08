#pragma once

#include <QWidget>
#include <QMap>

class QVBoxLayout;
class Ques;
class QXmlStreamWriter;
class QDomElement;

namespace Ui {
class EditView;
}

/**
 * @brief 编辑界面
 */
class EditView : public QWidget
{
    Q_OBJECT
public:
    explicit EditView(QWidget *parent = nullptr);
    ~EditView() override;

    /**
     * @brief   用于将Ques创建并添加到listWidget中
     * @param   pMetaObject 对象的QMetaObject
     * @return  创建的Ques
     */
    Ques* createQues(const QMetaObject *pMetaObject);

    /**
     * @brief   用于更新题目序号
     * @param   ind     题目索引
     * @param   val     -1表示默认序号，其他值则为给定序号
     */
    void updateIndex(int ind, int val = -1);

    /**
     * @brief   使用XML方式对题目列表进行保存
     * @param   xml     QXmlStreamWriter对象，用于写入XML
     */
    void writeQuesXml(QXmlStreamWriter &xml);
    /**
     * @brief   使用XML方式对题目列表进行读取
     * @param   elem    QDomElement对象，用于读取XML
     */
    void readQuesXml(const QDomElement &elem);

    /**
     * @brief   清空题目
     */
    void clearQues();

public slots:
    /**
     * @brief   响应右键菜单事件
     * @param   pos     位置
     */
    void onCustomContextMenuRequested(const QPoint &pos);

    /** @brief  响应添加题目事件 */
    void onAddClicked();

private:
    Ui::EditView *ui;
    QVBoxLayout *mLayoutScrollItems;

    struct QuesType {
        QString name;
        const QMetaObject *pMetaObject;
    };

    QMap<QString, QuesType> availableQues;
};
