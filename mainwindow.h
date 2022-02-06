#pragma once

#include <QMainWindow>

class QStackedLayout;
class MainView;

namespace Ui {
    class MainWindow;
}

/**
 * @brief   主窗口控件
 * 使用QStackedLayout对显示的子控件进行控制
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief   新建试卷
     * @param   filePath    新建的文件路径
     */
    void newProj(const QString &filePath);
    /**
     * @brief   读取试卷
     * @param   filePath    读取的文件路径
     */
    void loadProj(const QString &filePath);

public slots:
    /** @brief  用于响应新建试卷的信号 */
    void onNewProj();
    /** @brief  用于响应读取试卷的信号 */
    void onLoadProj();
    /** @brief  用于响应"关于"的信号 */
    void onAbout();
    /** @brief  用于响应"关于Qt"的信号 */
    void onAboutQt();

private:
    Ui::MainWindow *ui;
    QStackedLayout *mStkLayout;

    QString mProjPath;

    MainView *mMainView;
};
