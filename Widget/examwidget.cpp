#include "examwidget.h"
#include "ui_examwidget.h"

#include <QScrollBar>

#include <QTimer>
#include <QCryptographicHash>

#include <QDomDocument>
#include <QXmlStreamWriter>
#include <QTextStream>

#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>

#include "Util/header.h"
#include "Ques/quessinglechoice.h"
#include "Ques/quesmultichoice.h"
#include "Ques/queswhether.h"

ExamWidget::ExamWidget(const QString &dirName, bool hasEnd, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExamWidget),
      mUdpSocket(new QUdpSocket(this)), mTcpServer(new QTcpServer(this)),
      mTimeTimer(new QTimer(this)),
      mDirName(dirName), mDirPath(APP_DIR + "/Exported/" + dirName), mHasEnd(hasEnd),
      availableQues({
                    {"QuesSingleChoice", &QuesSingleChoiceData::staticMetaObject},
                    {"QuesMultiChoice", &QuesMultiChoiceData::staticMetaObject},
                    {"QuesWhether", &QuesWhetherData::staticMetaObject}
                    })
{
    ui->setupUi(this);

    QFile fileExported(mDirPath + "/_.esep");
    if(fileExported.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc;
        do {
            if(!doc.setContent(&fileExported))
                break;
            QDomElement root = doc.documentElement();
            if(root.tagName() != "ExamSysExportedProject")
                break;
            QDomElement elemQuesList, elemStuList;
            QDomNode node = root.firstChild();
            while(!node.isNull()) {
                QDomElement elem = node.toElement();
                if(!elem.isNull()) {
                    if(elem.tagName() == "QuesList") {
                        elemQuesList = elem;
                    } else if(elem.tagName() == "StuList") {
                        elemStuList = elem;
                    }
                }
                node = node.nextSibling();
            }
            if(elemQuesList.isNull() || elemStuList.isNull())
                break;

            // 读取信息
            const QString dateTimeFmt = "yyyy/M/d H:m:s";
            mName = root.attribute("Name");
            mDateTimeStart = QDateTime::fromString(root.attribute("StartDateTime"), dateTimeFmt);
            mDateTimeEnd = QDateTime::fromString(root.attribute("EndDateTime"), dateTimeFmt);
            mScoreInClient = root.attribute("ScoreInClient").toInt();

            // 读取题目列表
            node = elemQuesList.firstChild();
            while(!node.isNull()) {
                QDomElement elem = node.toElement();
                if(!elem.isNull()) {
                    auto iter = availableQues.constFind(elem.tagName());
                    do {
                        if(iter == availableQues.cend())
                            break;
                        QuesData *data = (QuesData*)iter.value()->newInstance(Q_ARG(QObject*, this));
                        if(!data)
                            break;
                        data->readXml(elem);
                        mListQues << data;
                    } while(false);
                }
                node = node.nextSibling();
            }

            // 读取考生列表
            node = elemStuList.firstChild();
            while(!node.isNull()) {
                QDomElement elem = node.toElement();
                if(!elem.isNull() && elem.tagName() == "v") {
                    QString name = elem.text();
                    if(!name.isEmpty())
                        mListStu << Stu{ name, elem.attribute("Pwd") };
                }
                node = node.nextSibling();
            }

            mError = NoError;
        } while(false);
    }

    if(mError != NoError)
        return;

    // 配置 udp
    if(!mUdpSocket->bind(40565, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        mError = UdpBindError;
        return;
    }
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &ExamWidget::onUdpReadyRead);

    // 配置 tcp
    if(!mTcpServer->listen()) {
        mError = TcpListenError;
        return;
    }
    connect(mTcpServer, &QTcpServer::newConnection, this, &ExamWidget::onNewConnection);

    // 获取本机IP
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for(const QHostAddress &address : qAsConst(addresses)) {
        if(address != QHostAddress::LocalHost && address.toIPv4Address()) {
            mAddress = address;
            break;
        }
    }
    if(mAddress.isNull())
        mAddress = QHostAddress::LocalHost;
    mMulticastAddress = QHostAddress("239.255.43.21");

    // 配置控件
    ui->labelExamName->setText(mName);
    ui->labelStartDateTime->setText(mDateTimeStart.toString("yyyy/M/d HH:mm:ss"));
    ui->labelEndDateTime->setText(mDateTimeEnd.toString("yyyy/M/d HH:mm:ss"));
    ui->labelScoreInClient->setText(mScoreInClient ? "是" : "否");

    // 初始化表头
    const QStringList horHeaderText = {
        "序号", "考生", "连接情况", "答题进度", "最后一次上传时间", "成绩"
    };
    ui->tableWidget->setColumnCount(horHeaderText.size());
    int i = 0;
    for(const QString &text : horHeaderText) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(text));
        i++;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // 添加表格物件
    ui->tableWidget->setRowCount(mListStu.size());
    i = 0;
    for(const Stu &stu : mListStu) {
        QTableWidgetItem *itemInd = new QTableWidgetItem(QString::number(i + 1));
        itemInd->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 0, itemInd);

        QTableWidgetItem *itemName = new QTableWidgetItem(stu.name);
        itemName->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 1, itemName);

        QTableWidgetItem *itemConnection = new QTableWidgetItem("未连接");
        itemConnection->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 2, itemConnection);

        QTableWidgetItem *itemProgress = new QTableWidgetItem("0%");
        itemProgress->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 3, itemProgress);

        QTableWidgetItem *itemLastUpload = new QTableWidgetItem("暂无");
        itemLastUpload->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 4, itemLastUpload);

        QTableWidgetItem *itemScore = new QTableWidgetItem("暂无");
        itemScore->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 5, itemScore);

        i++;
    }

    mTimeTimer->start(1000);
    connect(mTimeTimer, &QTimer::timeout, this, &ExamWidget::onTimeTimerTimeout);

    ui->listWidgetLog->setVisible(false);
    connect(ui->btnShowLog, &QPushButton::clicked, this, &ExamWidget::onSwitchLogVisible);

    setWindowTitle(mName);
    updateState();
}

ExamWidget::~ExamWidget() {
    for(auto iter = mMapStuClient.cbegin(); iter != mMapStuClient.cend(); ++iter) {
        iter.key()->blockSignals(true);
        iter.key()->disconnectFromHost();
        iter.key()->blockSignals(false);
    }
}

void ExamWidget::setIsConnected(const QString &stuName, bool isConnected) {\
    // 遍历每一行
    int rowCount = ui->tableWidget->rowCount();
    for(int i = 0; i < rowCount; ++i) {
        // 找到考生名字相符的
        if(ui->tableWidget->item(i, 1)->text() == stuName) {
            // 设置连接状态文字
            ui->tableWidget->item(i, 2)->setText(isConnected ? "已连接" : "未连接");
            break;
        }
    }
}

void ExamWidget::updateState() {
    QDateTime currentTime = QDateTime::currentDateTime();
    ui->labelCurTime->setText(currentTime.toString("yyyy/M/d HH:mm:ss"));
    if(mHasEnd || currentTime > mDateTimeEnd) {
        ui->labelState->setText("已结束");
        return;
    }
    if(currentTime < mDateTimeStart) {
        ui->labelState->setText("未开始");
        return;
    }
    ui->labelState->setText("进行中");
}

void ExamWidget::log(const QString &what) {
    QDateTime dateTime = QDateTime::currentDateTime();
    QString text = '[' + dateTime.toString("yyyy/M/d HH:mm:ss") + ']' + what;
    // 显示控件
    ui->listWidgetLog->addItem(text);
    QScrollBar *scroll = ui->listWidgetLog->verticalScrollBar();
    if(scroll->maximum() - scroll->value() < 3)
        ui->listWidgetLog->setCurrentRow(ui->listWidgetLog->count() - 1);
    // 写入文件
    QFile file(mDirPath + "/_.log");
    if(file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream(&file) << '\n' << text;
        file.close();
    }
}
void ExamWidget::log(const QTcpSocket *client, const QString &what) {
    log(QString("[%1:%2]").arg(client->peerAddress().toString()).arg(client->peerPort()) + what);
}

void ExamWidget::onSwitchLogVisible() {
    if(ui->listWidgetLog->isVisible()) {
        ui->btnShowLog->setText("显示日志");
        ui->listWidgetLog->setVisible(false);
    } else {
        ui->btnShowLog->setText("隐藏日志");
        ui->listWidgetLog->setVisible(true);
    }
}

void ExamWidget::onTimeTimerTimeout() {
    updateState();
    ++mMulticastSecCounter;
    if(mMulticastSecCounter > 10){
        mMulticastSecCounter = 0;
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "UpdTime");
        xml.writeAttribute("Address", mAddress.toString());
        xml.writeAttribute("Port", QString::number(mTcpServer->serverPort()));
        xml.writeCharacters(QDateTime::currentDateTime().toString("yyyy/M/d H:m:s"));
        xml.writeEndElement();
        xml.writeEndDocument();
        mUdpSocket->writeDatagram(array, mMulticastAddress, 40565);
    }
}

bool ExamWidget::parseUdpDatagram(const QByteArray &array) {
    QDomDocument doc;
    if(!doc.setContent(array))
        return false;
    QDomElement root = doc.documentElement();
    if(root.tagName() != "ESDtg")
        return false;

    QString type = root.attribute("Type");
    if(type == "SearchServer") {
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "SearchServerRetval");
        xml.writeAttribute("Address", mAddress.toString());
        xml.writeAttribute("Port", QString::number(mTcpServer->serverPort()));
        xml.writeCharacters(ui->labelExamName->text());
        xml.writeEndElement();
        xml.writeEndDocument();
        mUdpSocket->writeDatagram(array, QHostAddress(root.text()), 40565);
    } else return false;

    return true;
}

bool ExamWidget::parseTcpDatagram(QTcpSocket *client, const QByteArray &array) {
    QDomDocument doc;
    if(!doc.setContent(array))
        return false;
    QDomElement root = doc.documentElement();
    if(root.tagName() != "ESDtg")
        return false;

    QString type = root.attribute("Type");
    if(type == "ExamDataRequest") {
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "ExamData");

        // 写入名称
        xml.writeAttribute("Name", ui->labelExamName->text());

        // 写入时间
        const QString dateTimeFmt = "yyyy/M/d H:m:s";
        xml.writeAttribute("StartDateTime", mDateTimeStart.toString(dateTimeFmt));
        xml.writeAttribute("EndDateTime", mDateTimeEnd.toString(dateTimeFmt));
        xml.writeAttribute("CurDateTime", QDateTime::currentDateTime().toString(dateTimeFmt));

        // 写入试卷列表
        xml.writeStartElement("QuesList");
        for(const QuesData *ques : mListQues) {
            ques->writeXmlWithoutTrueAns(xml);
        }
        xml.writeEndElement();

        xml.writeEndElement();
        xml.writeEndDocument();
        qint64 ret = tcpSendDatagram(client, array);
        log(client, QString("传输试卷 长度:%1 发送:%2").arg(array.length() + 4).arg(ret));
    } else return false;

    return true;
}

qint64 ExamWidget::udpSendVerifyErr(const QString &what, const QHostAddress &address) {
    QByteArray array;
    QXmlStreamWriter xml(&array);
    xml.writeStartDocument();
    xml.writeStartElement("ESDtg");
    xml.writeAttribute("Type", "VerifyErr");
    xml.writeCharacters(what);
    xml.writeEndElement();
    xml.writeEndDocument();
    return mUdpSocket->writeDatagram(array, address, 40565);
}

qint64 ExamWidget::tcpSendDatagram(QTcpSocket *client, const QByteArray &array) {
    int len = array.length();
    return client->write(QByteArray((char*)&len, 4) + array);
}

void ExamWidget::onUdpReadyRead() {
    QByteArray datagram;
    while (mUdpSocket->hasPendingDatagrams()) {
        datagram.resize(int(mUdpSocket->pendingDatagramSize()));
        mUdpSocket->readDatagram(datagram.data(), datagram.size());
        parseUdpDatagram(datagram);
    }
}

void ExamWidget::onNewConnection() {
    QTcpSocket *client = mTcpServer->nextPendingConnection();
    QString stuName;
    {   // 验证密码
        QObject obj;            // 用于临时槽函数
        QEventLoop eventLoop;   // 用于阻塞执行
        bool verified = false;

        // 6s超时
        QTimer::singleShot(6000, &obj, [&eventLoop] { eventLoop.quit(); });
        // 绑定信号与槽接收消息，进行验证
        connect(client, &QTcpSocket::readyRead, &obj, [this, client, &stuName, &eventLoop, &verified] {
            do {
                // 读取xml
                QDomDocument doc;
                if(!doc.setContent(client->readAll()))
                    break;
                QDomElement root = doc.documentElement();
                if(root.tagName() != "ESDtg" || root.attribute("Type") != "TcpVerify")
                    break;

                // 判断考生是否存在
                stuName = root.attribute("StuName");
                const Stu *stu = findStu(stuName);
                if(!stu) {
                    udpSendVerifyErr("考生不存在", client->peerAddress());
                    break;
                }

                // 验证密码
                uint salt = root.attribute("Salt").toUInt();
                QByteArray verify;
                QDataStream ds(&verify, QIODevice::WriteOnly);
                ds << client->peerAddress().toIPv4Address() << client->peerPort() << stu->pwd << salt;
                if(QCryptographicHash::hash(verify, QCryptographicHash::Md5).toHex() != root.text()) {
                    udpSendVerifyErr("密码错误", client->peerAddress());
                    break;
                }

                verified = true;
            } while(false);

            eventLoop.quit();
        });

        eventLoop.exec();

        // 如果验证失败，则使client断开并结束函数调用
        if(!verified) {
            log(client, stuName.isEmpty() ? "连入 验证失败" : "连入 \"" + stuName + "\" 验证失败");
            client->disconnectFromHost();
            return;
        }
        log(client, stuName.isEmpty() ? "连入 验证成功" : "连入 \"" + stuName + "\" 验证成功");

        // 发送验证成功消息
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "VerifySucc");
        xml.writeEndElement();
        xml.writeEndDocument();
        tcpSendDatagram(client, array);
    }

    // 设置相关内容
    mMapStuClient[client] = stuName;
    setIsConnected(stuName, true);
    connect(client, &QTcpSocket::readyRead, this, &ExamWidget::onTcpReadyRead);
    connect(client, &QTcpSocket::disconnected, this, [this, client, stuName] {
        mMapStuClient.remove(client);
        setIsConnected(stuName, false);
        log(client, "\"" + stuName + "\" 断开连接");
    });
}

void ExamWidget::onTcpReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    Client &c = mMapStuClient[client];
    c.buffer += client->readAll();
    if(c.buffer.length() < 4)
        return;
    int len = *reinterpret_cast<int*>(c.buffer.data());
    while(c.buffer.length() >= 4 + len) {
        parseTcpDatagram(client, c.buffer.mid(4, len));
        c.buffer.remove(0, 4 + len);
    }
}

const ExamWidget::Stu* ExamWidget::findStu(const QString &name) {
    for(const Stu &stu : mListStu) {
        if(stu.name == name)
            return &stu;
    }
    return nullptr;
}

