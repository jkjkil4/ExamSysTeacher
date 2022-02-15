#pragma once

#include <QWidget>

namespace Ui {
class PushView;
}

class EditView;
class QXmlStreamWriter;

/**
 * @brief   发放试卷页面
 */
class PushView : public QWidget
{
    Q_OBJECT

public:
    explicit PushView(EditView *editView, QWidget *parent = nullptr);
    ~PushView();

    /**
     * @brief   自动调整答题的起止时间
     * 调整方式:
     *      开始时间 设置为当前该时间的下一个整点时间
     *      结束时间 设置为开始时间的一个小时后
     */
    void autoAdjustDateTime();

    /**
     * @brief   检查考生列表是否有重复元素
     * @param   name    考生名称
     * @return  是否重复
     */
    bool listStuContains(const QString &name);

    /**
     * @brief   设置项目名称用于显示
     * @param   projName    项目名称
     */
    void setProjName(const QString &projName);

    /**
     * @brief   通过XML写入发放属性
     * @param   xml     QXmlStreamWriter对象
     */
    void writeAttributesXml(QXmlStreamWriter &xml);
    /**
     * @brief   通过XML写入学生列表
     * @param   xml     QXmlStreamWriter对象
     */
    void writeStuListXml(QXmlStreamWriter &xml);

public slots:
    /** @brief  响应导入考生列表操作 */
    void onImport();
    /** @brief  响应发放试卷操作 */
    void onPush();

signals:
    void back();
    void exam(const QString &dirName);

private:
    Ui::PushView *ui;
    EditView *mEditView;

    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
};
