#include "doubleslidebutton.h"

#include <QPainter>
#include <QtMath>

DoubleSlideButton::DoubleSlideButton()
{
    connect(this, &DoubleSlideButton::clicked, this, &DoubleSlideButton::onClicked);
    updateSizeLimit();
}

void DoubleSlideButton::updateSizeLimit() {
    int horLim = 2 * (mCircleRadius + mBorderSize) + mRectLength;
    int verLim = 2 * (mCircleRadius + mBorderSize);
    QSize size = QSize(horLim, verLim);
    setMinimumSize(size);
    setMaximumSize(size);
}

void DoubleSlideButton::onClicked() {
    QPoint pos = mapFromGlobal(cursor().pos());
    mState = pos.x() < mCircleRadius + mRectLength / 2 ? Left : Right;
    update();
}

void DoubleSlideButton::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QPen(mColorBorder, mBorderSize));
    switch(mState) {
    case Left: p.setBrush(mColorBackgroundLeft); break;
    case Mid: p.setBrush(mColorBackgroundMid); break;
    case Right: p.setBrush(mColorBackgroundRight); break;
    }

    QPainterPath borderPath;
    double half = mBorderSize / 2.0;
    int outerCircleLen = 2 * (mCircleRadius + mBorderSize);
    borderPath.moveTo(mBorderSize + mCircleRadius, half);
    borderPath.arcTo(QRectF(0, 0, outerCircleLen, outerCircleLen).adjusted(half, half, -half, -half), 90, 180);
    borderPath.lineTo(mBorderSize + mCircleRadius + mRectLength, outerCircleLen - half);
    borderPath.arcTo(QRectF(mRectLength, 0, outerCircleLen, outerCircleLen).adjusted(half, half, -half, -half), 270, 180);
    borderPath.closeSubpath();
    p.drawPath(borderPath);

    p.setBrush(mColorButton);
    switch(mState) {
    case Left: p.drawEllipse(QRectF(0, 0, outerCircleLen, outerCircleLen).adjusted(half, half, -half, -half)); break;
    case Mid: p.drawEllipse(QRectF(mRectLength / 2.0, 0, outerCircleLen, outerCircleLen).adjusted(half, half, -half, -half)); break;
    case Right: p.drawEllipse(QRectF(mRectLength, 0, outerCircleLen, outerCircleLen).adjusted(half, half, -half, -half)); break;
    }
}

int DoubleSlideButton::circleRadius() const { return mCircleRadius; }
void DoubleSlideButton::setCircleRadius(int circleRadius) {
    if(circleRadius != mCircleRadius) {
        mCircleRadius = circleRadius;
        updateSizeLimit();
    }
}
int DoubleSlideButton::rectLength() const { return mRectLength; }
void DoubleSlideButton::setRectLength(int rectLength) {
    if(rectLength != mRectLength) {
        mRectLength = rectLength;
        updateSizeLimit();
    }
}
QColor DoubleSlideButton::colorBorder() const { return mColorBorder; }
void DoubleSlideButton::setColorBorder(const QColor &colorBorder) {
    if(colorBorder != mColorBorder) {
        mColorBorder = colorBorder;
        update();
    }
}
QColor DoubleSlideButton::colorBackgroundMid() const { return mColorBackgroundMid; }
void DoubleSlideButton::setColorBackgroundMid(const QColor &colorBackgroundMid) {
    if(colorBackgroundMid != mColorBackgroundMid) {
        mColorBackgroundMid = colorBackgroundMid;
        update();
    }
}
QColor DoubleSlideButton::colorBackgroundLeft() const { return mColorBackgroundLeft; }
void DoubleSlideButton::setColorBackgroundLeft(const QColor &colorBackgroundLeft) {
    if(colorBackgroundLeft != mColorBackgroundLeft) {
        mColorBackgroundLeft = colorBackgroundLeft;
        update();
    }
}
QColor DoubleSlideButton::colorBackgroundRight() const { return mColorBackgroundRight; }
void DoubleSlideButton::setColorBackgroundRight(const QColor &colorBackgroundRight) {
    if(colorBackgroundRight != mColorBackgroundRight) {
        mColorBackgroundRight = colorBackgroundRight;
        update();
    }
}
QColor DoubleSlideButton::colorButton() const { return mColorButton; }
void DoubleSlideButton::setColorButton(const QColor &colorButton)
{
    if(colorButton != mColorButton) {
        mColorButton = colorButton;
        update();
    }
}
DoubleSlideButton::State DoubleSlideButton::state() const { return mState; }
void DoubleSlideButton::setState(const State &state) {
    if(state != mState) {
        mState = state;
        emit stateChanged();
        update();
    }
}

