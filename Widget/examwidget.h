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
class QTcpSocket;

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
    ~ExamWidget() override;

    Error error() { return mError; }

    /**
     * @brief   根据时间关系更新状态
     */
    void updateState();

    void setIsConnected(const QString &stuName, bool isConnected);

    void udpSendVerifyErr(const QString &what, const QHostAddress &address);
    void udpSendVerifySucc(const QHostAddress &address);

public slots:
    /** @brief  用于响应udp接收消息事件 */
    void onUdpReadyRead();

public:
    void onUdpReadyRead_SearchServer(const QDomElement &elem);

public slots:
    /** @brief  用于响应tcp连入消息 */
    void onNewConnection();

private:
    // 界面
    Ui::ExamWidget *ui;

    // 网络Socket
    QUdpSocket *mUdpSocket;
    QTcpServer *mTcpServer;

    // 目录相关
    QString mDirName, mDirPath;

    // 基本属性
    bool mHasEnd;
    Error mError = NormalError;
    QHostAddress mAddress;

    // 考试信息
    QString mName;
    QDateTime mDateTimeStart, mDateTimeEnd;
    bool mScoreInClient;

    // 题目列表
    QMap<QString, const QMetaObject *> availableQues;
    QVector<QuesData*> mListQues;

    // 学生信息
    struct Stu { QString name, pwd; };
    QList<Stu> mListStu;
    const Stu* findStu(const QString &name);

    // 学生客户端
    QMap<QTcpSocket*, QString> mMapStuClient;
};
