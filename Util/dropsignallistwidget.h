#pragma once

#include <QListWidget>

/**
 * @brief 在拖放后发出信号的QListWidget
 */
class DropSignalListWidget : public QListWidget
{
    Q_OBJECT
public:
    using QListWidget::QListWidget;

signals:
    void drop();

protected:
    void dropEvent(QDropEvent *ev) override {
        QListWidget::dropEvent(ev);
        emit drop();
    }
};
