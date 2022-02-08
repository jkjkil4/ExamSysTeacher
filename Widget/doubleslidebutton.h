#pragma once

#include <QAbstractButton>

/**
 * @brief   双向滑动按钮
 */
class DoubleSlideButton : public QAbstractButton
{
    Q_OBJECT
public:
    enum State { Left, Mid, Right };

    DoubleSlideButton();

    /**
     * @brief   更新控件大小限制
     */
    void updateSizeLimit();

public slots:
    void onClicked();

protected:
    void paintEvent(QPaintEvent *) override;

public:
    int circleRadius() const;
    void setCircleRadius(int circleRadius);

    int rectLength() const;
    void setRectLength(int rectLength);

    QColor colorBorder() const;
    void setColorBorder(const QColor &colorBorder);

    QColor colorBackgroundMid() const;
    void setColorBackgroundMid(const QColor &colorBackgroundMid);

    QColor colorBackgroundLeft() const;
    void setColorBackgroundLeft(const QColor &colorBackgroundLeft);

    QColor colorBackgroundRight() const;
    void setColorBackgroundRight(const QColor &colorBackgroundRight);

    QColor colorButton() const;
    void setColorButton(const QColor &colorButton);

    State state() const;
    void setState(const State &state);

signals:
    void stateChanged();

private:
    int mCircleRadius = 12;
    int mRectLength = 32;
    int mBorderSize = 3;

    QColor mColorBorder = QColor(24, 204, 192);
    QColor mColorBackgroundMid = QColor(240, 240, 240);
    QColor mColorBackgroundLeft = QColor(250, 140, 140);
    QColor mColorBackgroundRight = QColor(140, 250, 140);
    QColor mColorButton = Qt::white;

    State mState = Mid;
};
