#pragma once

#include <QWidget>

namespace Ui {
    class MainView;
}

/**
 * @brief   窗口主界面
 * 包含新建试卷、加载试卷的按钮
 */
class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(QWidget *parent = nullptr);

signals:
    void newProjClicked();
    void loadProjClicked();

private:
    Ui::MainView *ui;
};

