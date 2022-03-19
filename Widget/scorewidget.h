#pragma once

#include <QWidget>

class QDomElement;

class QToolBar;
class QAction;
class QLabel;
class QScrollArea;
class QVBoxLayout;

class ScoreWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScoreWidget(const QDomElement &elem, QWidget *parent = nullptr);

    /**
     * @brief   得到当前位置对应的LayoutItem的索引
     * @return  索引，-1表示无效
     */
    int currentInd(bool ignoreTop);

public slots:
    void onPrevWrong();
    void onNextWrong();

private:
    QToolBar *mToolBar;
    QAction *mActPrevWrong, *mActNextWrong;
    QLabel *mLabelExam, *mLabelStu, *mLabelScore;

    QScrollArea *mScrollArea;
    QWidget *mScrollAreaWidget;
    QVBoxLayout *mLayout, *mLayoutWidgets;

    QVector<bool> mVecIsRight;
};
