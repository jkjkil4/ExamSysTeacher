#pragma once

#include <QListWidget>

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
