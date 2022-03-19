#pragma once

#include <QWidget>

#include <QFile>
#include <QDateTime>
#include <QMap>
#include <QHostAddress>
#include <QLockFile>
#include <QSettings>

namespace Ui {
class ExamWidget;
}

class QTableWidgetItem;

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
        NetworkError = UdpBindError | TcpListenError,
        FileLockError = 0x08
    };
    Q_ENUM(Error)

    explicit ExamWidget(const QString &dirName, bool hasEnd = false, QWidget *parent = nullptr);
    ~ExamWidget() override;

    Error error() { return mError; }

    int stuInd(const QString &stuName);
    int stuRow(const QString &stuName);
    void setStuIsConnected(const QString &stuName, bool isConnected);
    void setStuProc(const QString &stuName, int proc);
    void setStuUploadTime(const QString &stuName, const QDateTime &dt);
    void setStuScore(const QString &stuName, int right);

    /** @brief  根据时间关系更新状态 */
    void updateState();

    void log(const QString &what);
    void log(const QTcpSocket *client, const QString &what);

public slots:
    /** @brief  响应切换日志是否可见 */
    void onSwitchLogVisible();
    /** @brief  响应定时器事件 */
    void onTimeTimerTimeout();
    /** @brief  响应item双击事件，显示具体改分情况 */
    void onItemDoubleClicked(QTableWidgetItem *item);

public:
    bool parseUdpDatagram(const QByteArray &array);
    bool parseTcpDatagram(QTcpSocket *client, const QByteArray &array);

    qint64 udpSendVerifyErr(const QString &what, const QHostAddress &address);

    qint64 tcpSendDatagram(QTcpSocket *client, const QByteArray &array);

public slots:
    /** @brief  用于响应udp接收消息事件 */
    void onUdpReadyRead();

public slots:
    /** @brief  用于响应tcp连入消息 */
    void onNewConnection();
    /** @brief  用于接收tcp消息 */
    void onTcpReadyRead();

protected:
    void closeEvent(QCloseEvent *ev) override;

private:
    // 界面
    Ui::ExamWidget *ui;

    // 网络Socket
    QUdpSocket *mUdpSocket;
    QTcpServer *mTcpServer;

    // 定时器，用于更新时间
    QTimer *mTimeTimer;

    // 目录相关
    QString mDirName, mDirPath;
    QLockFile mLockFile;
    QSettings mConfigFile;

    // 基本属性
    bool mHasEnd;
    Error mError = NormalError;
    QHostAddress mAddress;
    int mUpdTimeSecCounter = 0;

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

    // 考生客户端
    struct Client {
        Client() = default;
        Client(const QString &name) : stuName(name) {}
        QString stuName;
        QByteArray buffer;
    };
    QMap<QTcpSocket*, Client> mMapStuClient;
};
