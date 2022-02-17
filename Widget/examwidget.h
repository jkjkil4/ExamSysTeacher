#pragma once

#include <QWidget>

#include <QFile>
#include <QDateTime>
#include <QMap>
#include <QHostAddress>

namespace Ui {
class ExamWidget;
}

class QuesData;
class QDomElement;
class QUdpSocket;
class QTcpServer;

/**
 * @brief   考试控制窗口
 */
class ExamWidget : public QWidget
{
    Q_OBJECT
public:
    enum State { BeforeProc, Proc, AfterProc };
    enum Error {
        NoError = 0,
        NormalError = 0x01,
        UdpBindError = 0x02,
        TcpListenError = 0x04,
        NetworkError = UdpBindError | TcpListenError
    };
    Q_ENUM(Error)

    explicit ExamWidget(const QString &dirName, bool hasEnd = false, QWidget *parent = nullptr);

    Error error() { return mError; }

    /**
     * @brief   根据时间关系更新状态
     */
    void updateState();

    void onUdpReadyRead_SearchServer(const QDomElement &elem);

public slots:
    /** @brief  用于响应udp接收消息事件 */
    void onUdpReadyRead();

private:
    Ui::ExamWidget *ui;
    QUdpSocket *mUdpSocket;
    QTcpServer *mTcpServer;
    QString mDirName, mDirPath;
    bool mHasEnd;

    Error mError = NormalError;
    QHostAddress mAddress;

    QString mName;
    QDateTime mDateTimeStart, mDateTimeEnd;
    bool mScoreInClient;

    QMap<QString, const QMetaObject *> availableQues;
    QVector<QuesData*> mListQues;

    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
};
