#pragma once

#include <QMainWindow>

class QStackedLayout;
class MainView;
class EditView;
class PushView;

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
    ~MainWindow() override;

    /**
     * @brief   新建试卷
     * @param   filePath    新建的文件路径
     */
    bool newProj(const QString &filePath);
    /**
     * @brief   读取试卷
     * @param   filePath    读取的文件路径
     */
    bool loadProj(const QString &filePath);
    /**
     * @brief   保存试卷
     * @param   filePath    保存的文件路径
     * @return
     */
    bool saveProj(const QString &filePath);

    bool displayExam(const QString &dirName);

    bool verifyClose();

public slots:
    /** @brief  用于响应新建试卷的信号 */
    void onNewProj();
    /** @brief  用于响应读取试卷的信号 */
    void onLoadProj();
    /** @brief  用于响应保存试卷的信号 */
    void onSaveProj();

    /** @brief  用于响应显示历次考试的信号 */
    void onDisplayExamHistory();

    /** @brief  用于响应发放试卷信号（进入PushView） */
    void onPush();
    /** @brief  用于响应发放试卷控件中的返回按钮 */
    void onPushViewBack();
    /**
     * @brief   用于响应发放试卷信号（创建ExamWidget）
     * @param   dirName     目录名
     */
    void onExam(const QString &dirName);

    /** @brief  用于响应"关于"的信号 */
    void onAbout();
    /** @brief  用于响应"关于Qt"的信号 */
    void onAboutQt();

protected:
    void closeEvent(QCloseEvent *ev) override;

private:    
    Ui::MainWindow *ui;
    QStackedLayout *mStkLayout;

    QString mProjPath;
    bool mIsChanged = false;

    MainView *mMainView;
    EditView *mEditView;
    PushView *mPushView;
};
