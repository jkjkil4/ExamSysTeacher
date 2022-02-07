#pragma once

#include <QWidget>
#include <QMap>

class QListWidgetItem;

namespace Ui {
class EditView;
}

class EditView : public QWidget
{
    Q_OBJECT
public:
    explicit EditView(QWidget *parent = nullptr);
    ~EditView() override;

    /**
     * @brief   用于将Ques创建并添加到listWidget中
     * @param   pMetaObject 对象的QMetaObject
     */
    void createQues(const QMetaObject *pMetaObject);

public slots:
    /**
     * @brief   响应双击item事件
     * @param   双击的item
     */
    void onItemDoubleClicked(QListWidgetItem *item);
    /**
     * @brief   用于更新题目的序号
     */
    void updateIndex();

private:
    Ui::EditView *ui;

    struct QuesType {
        QString name;
        const QMetaObject *pMetaObject;
    };

    QMap<QString, QuesType> availableQues;
};
