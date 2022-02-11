#pragma once

#include <QWidget>

namespace Ui {
class PushView;
}

/**
 * @brief   发放试卷页面
 */
class PushView : public QWidget
{
    Q_OBJECT

public:
    explicit PushView(QWidget *parent = nullptr);
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

public slots:
    /** @brief  响应导入考生列表操作 */
    void onImport();

signals:
    void back();

private:
    Ui::PushView *ui;

    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
};
