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

private:
    Ui::PushView *ui;
};
