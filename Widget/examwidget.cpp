#include "examwidget.h"
#include "ui_examwidget.h"

#include <QScrollBar>
#include <QMessageBox>
#include <QCloseEvent>

#include <QTimer>
#include <QCryptographicHash>

#include <QDomDocument>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextStream>

#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QNetworkDatagram>

#include "Util/header.h"
#include "Ques/quessinglechoice.h"
#include "Ques/quesmultichoice.h"
#include "Ques/queswhether.h"
#include "Widget/scorewidget.h"

ExamWidget::ExamWidget(const QString &dirName, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExamWidget),
      mUdpSocket(new QUdpSocket(this)), mTcpServer(new QTcpServer(this)),
      mTimeTimer(new QTimer(this)),
      mDirName(dirName), mDirPath(APP_DIR + "/Exported/" + dirName),
      mLockFile(mDirPath + "/_.lock"), mConfigFile(mDirPath + "/_.ini", QSettings::IniFormat),
      availableQues({
                    {"QuesSingleChoice", &QuesSingleChoiceData::staticMetaObject},
                    {"QuesMultiChoice", &QuesMultiChoiceData::staticMetaObject},
                    {"QuesWhether", &QuesWhetherData::staticMetaObject}
                    })
{
    ui->setupUi(this);

    if(!mLockFile.tryLock()) {
        mError = FileLockError;
        return;
    }

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
            ui->cbbScoreInClient->setChecked(root.attribute("ScoreInClient").toInt());

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
        fileExported.close();
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

    // 配置控件
    ui->labelExamName->setText(mName);
    ui->labelStartDateTime->setText(mDateTimeStart.toString("yyyy/M/d HH:mm:ss"));
    ui->labelEndDateTime->setText(mDateTimeEnd.toString("yyyy/M/d HH:mm:ss"));
    ui->labelNetwork->setText(mAddress.toString() + ":" + QString::number(mTcpServer->serverPort()));

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

        QTableWidgetItem *itemProgress = new QTableWidgetItem;  //("0%");
        itemProgress->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 3, itemProgress);

        QTableWidgetItem *itemLastUpload = new QTableWidgetItem("暂无");
        itemLastUpload->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 4, itemLastUpload);

        QTableWidgetItem *itemScore = new QTableWidgetItem("暂无");
        itemScore->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        itemScore->setData(Qt::UserRole, i);
        ui->tableWidget->setItem(i, 5, itemScore);

        // 读取考生进度
        itemProgress->setText(mConfigFile.value(QString("Stu/%1_Proc").arg(i), "0").toString() + "%");
        // 读取考生成绩
        QString stuScoreKey = QString("Stu/%1_Score").arg(i);
        if(mConfigFile.contains(stuScoreKey))
            itemScore->setText(mConfigFile.value(stuScoreKey).toString() + "/" + QString::number(mListQues.size()));

        // 读取考生最后一次提交时间
        QFile file(mDirPath + "/" + QString::number(i) + ".stuans");
        do {
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
                break;
            QXmlStreamReader xml(&file);
            if(!xml.readNextStartElement())
                break;
            QDateTime dt = QDateTime::fromString(xml.attributes().value("Time").toString(), "yyyy/M/d H:m:s");
            if(!dt.isValid())
                break;
            itemLastUpload->setText(dt.toString("HH:mm:ss"));
        } while(false);
        if(file.isOpen())
            file.close();

        i++;
    }

    ui->listWidgetLog->setVisible(false);
    connect(ui->btnShowLog, &QPushButton::clicked, this, &ExamWidget::onSwitchLogVisible);

    mTimeTimer->start(1000);
    connect(mTimeTimer, &QTimer::timeout, this, &ExamWidget::onTimeTimerTimeout);

    connect(ui->tableWidget, &QTableWidget::itemDoubleClicked, this, &ExamWidget::onItemDoubleClicked);

    setWindowTitle(mName);
    updateState();
}

ExamWidget::~ExamWidget() {
    for(auto iter = mMapStuClient.cbegin(); iter != mMapStuClient.cend(); ++iter) {
        iter.key()->blockSignals(true);
        iter.key()->disconnectFromHost();
        iter.key()->blockSignals(false);
    }

    if(mLockFile.isLocked())
        mLockFile.unlock();
}

int ExamWidget::stuInd(const QString &stuName) {
    int i = 0;
    for(const Stu &stu : mListStu) {
        if(stu.name == stuName)
            return i;
        ++i;
    }
    return -1;
}
int ExamWidget::stuRow(const QString &stuName) {
    int rowCount = ui->tableWidget->rowCount();
    for(int i = 0; i < rowCount; ++i) {
        if(ui->tableWidget->item(i, 1)->text() == stuName)
            return i;
    }
    return -1;
}
void ExamWidget::setStuIsConnected(const QString &stuName, bool isConnected) {
    int row = stuRow(stuName);
    if(row == -1)
        return;
    ui->tableWidget->item(row, 2)->setText(isConnected ? "已连接" : "未连接");
}
void ExamWidget::setStuProc(const QString &stuName, int proc) {
    int row = stuRow(stuName);
    if(row == -1)
        return;
    ui->tableWidget->item(row, 3)->setText(QString::number(proc) + "%");
}
void ExamWidget::setStuUploadTime(const QString &stuName, const QDateTime &dt) {
    int row = stuRow(stuName);
    if(row == -1)
        return;
    ui->tableWidget->item(row, 4)->setText(dt.toString("HH:mm:ss"));
}
void ExamWidget::setStuScore(const QString &stuName, int right) {
    int row = stuRow(stuName);
    if(row == -1)
        return;
    ui->tableWidget->item(row, 5)->setText(QString::number(right) + "/" + QString::number(mListQues.size()));
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

ExamWidget::ScoreResult ExamWidget::score(const QString &stuName, bool *ok) {
    int ind = stuInd(stuName);
    // 尝试打开文件
    QFile file(mDirPath + "/" + QString::number(ind) + ".stuans");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if(ok) *ok = false;
        log("\"" + stuName + "\" 改分 失败（文件无法访问）");
        return ScoreResult{};
    }
    // 尝试转为QDomDocument
    QDomDocument docAns;
    if(!docAns.setContent(&file)) {
        if(ok) *ok = false;
        log("\"" + stuName + "\" 改分 失败（文件无效）");
        return ScoreResult{};
    }

    // 得到改分信息
    QList<QuesData::Score> scoreList;
    int quesCnt = mListQues.size(), quesRight = 0;
    QDomNode node = docAns.documentElement().firstChild();
    int i = 0;
    while(!node.isNull() && i < quesCnt) {
        QDomElement elem = node.toElement();
        if(!elem.isNull()) {
            QuesData::Score score = mListQues[i]->score(elem.text());
            scoreList << score;
            if(score.isRight)
                ++quesRight;
            ++i;
        }
        node = node.nextSibling();
    }
    // 设置得分
    setStuScore(stuName, quesRight);
    mConfigFile.setValue(QString("Stu/%1_Score").arg(ind), QString::number(quesRight));
    mConfigFile.setValue(QString("Stu/%1_Scored").arg(ind), true);

    // 输出日志
    log("\"" + stuName + "\" 改分 成功");

    if(ok) *ok = true;
    return ScoreResult{ quesRight, scoreList };
}

void ExamWidget::scoreAll() {
    int i = 0;
    for(auto iter = mListStu.cbegin(); iter != mListStu.cend(); ++iter, ++i) {
        QString str = QString::number(i);

        // 如果已经改过分，则忽略
        if(!mConfigFile.value(QString("Stu/%1_Scored").arg(i), false).toBool())
            continue;

        QFile ansFile(mDirPath + "/" + str + ".stuans");
        // 如果不存在作答，则忽略
        if(!ansFile.exists())
            continue;

        bool ok;
        ScoreResult sr = score(iter->name, &ok);
        if(ok) {
            // 将改分信息写入文件
            QFile fileScore(mDirPath + "/" + QString::number(i) + ".stuscore");
            if(fileScore.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xml(&fileScore);
                xml.setAutoFormatting(true);
                xml.writeStartDocument();
                writeScoreResultToXml(iter->name, sr, xml);
                xml.writeEndDocument();
                fileScore.close();
            } else log("写入 \"" + iter->name + "\" 得分文件失败");

            // 得到对应客户端
            QTcpSocket *client = nullptr;
            for(auto iter2 = mMapStuClient.cbegin(); iter2 != mMapStuClient.cend(); ++iter2) {
                if(iter2.value().stuName == iter->name) {
                    client = iter2.key();
                    break;
                }
            }
            // 发送改分信息
            if(client) {
                QByteArray array;
                QXmlStreamWriter xml(&array);
                xml.writeStartDocument();
                xml.writeStartElement("ESDtg");
                xml.writeAttribute("Type", "StuFinishRetval");
                if(ui->cbbScoreInClient->isChecked())
                    writeScoreResultToXml(iter->name, sr, xml);
                xml.writeEndElement();
                xml.writeEndDocument();
                tcpSendDatagram(client, array);
            }
        }
    }
}
void ExamWidget::writeScoreResultToXml(const QString &stuName, const ScoreResult &sr, QXmlStreamWriter &xml) {
    xml.writeStartElement("ScoreList");
    xml.writeAttribute("ExamName", ui->labelExamName->text());
    xml.writeAttribute("StuName", stuName);
    xml.writeAttribute("Score", QString::number(sr.quesRight));
    xml.writeAttribute("TotalScore", QString::number(mListQues.size()));
    for(const QuesData::Score &score : sr.scoreList) {
        xml.writeStartElement("v");
        xml.writeAttribute("Right", QString::number(score.isRight));
        xml.writeCharacters(score.html);
        xml.writeEndElement();
    }
    xml.writeEndElement();
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
    if(!mHasEnd && QDateTime::currentDateTime() >= mDateTimeEnd) {
        mHasEnd = true;

        // 改分
        scoreAll();

        // 考试结束信息
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "SrvClosed");
        xml.writeEndElement();
        xml.writeEndDocument();
        // 向未改分的客户端发送考试结束信息
        for(auto iter = mMapStuClient.cbegin(); iter != mMapStuClient.cend(); ++iter) {
            if(!mConfigFile.value(QString("Stu/%1_Scored").arg(stuInd(iter->stuName)), false).toBool())
                tcpSendDatagram(iter.key(), array);
        }
    }
    ++mUpdTimeSecCounter;
    if(mUpdTimeSecCounter >= 10){
        mUpdTimeSecCounter = 0;
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
        for(auto iter = mMapStuClient.cbegin(); iter != mMapStuClient.cend(); ++iter)
            tcpSendDatagram(iter.key(), array);
    }
}

void ExamWidget::onItemDoubleClicked(QTableWidgetItem *item) {
    if(item->column() != 5)
        return;
    int ind = item->data(Qt::UserRole).toInt();
    QFile file(mDirPath + "/" + QString::number(ind) + ".stuscore");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QDomDocument doc;
    bool ret = doc.setContent(&file);
    file.close();
    if(!ret) return;

    ScoreWidget *widget = new ScoreWidget(doc.documentElement());
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
}

bool ExamWidget::parseUdpDatagram(const QNetworkDatagram &datagram) {
    QDomDocument doc;
    if(!doc.setContent(datagram.data()))
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
        xml.writeAttribute("ID", mDirName);
        xml.writeCharacters(ui->labelExamName->text());
        xml.writeEndElement();
        xml.writeEndDocument();
        mUdpSocket->writeDatagram(array, datagram.senderAddress(), (ushort)datagram.senderPort());
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
    QString stuName = mMapStuClient[client].stuName;
    if(type == "ExamDataRequest") {
        QByteArray array;
        QXmlStreamWriter xml(&array);
        xml.writeStartDocument();
        xml.writeStartElement("ESDtg");
        xml.writeAttribute("Type", "ExamData");

        // 写入信息
        xml.writeAttribute("ID", mDirName);
        xml.writeAttribute("Name", ui->labelExamName->text());

        // 写入时间
        const QString dateTimeFmt = "yyyy/M/d H:m:s";
        xml.writeAttribute("StartDateTime", mDateTimeStart.toString(dateTimeFmt));
        xml.writeAttribute("EndDateTime", mDateTimeEnd.toString(dateTimeFmt));
        xml.writeAttribute("CurDateTime", QDateTime::currentDateTime().toString(dateTimeFmt));

        // 写入试卷列表
        xml.writeStartElement("QuesList");
        int i = 0;
        for(const QuesData *ques : mListQues) {
            ques->writeXmlWithoutTrueAns(xml);
            ++i;
        }
        xml.writeEndElement();

        // 写入考生作答（直接传回之前上传的内容）
        if(root.attribute("StuAnsRequest").toInt()) {
            QFile file(mDirPath + "/" + QString::number(stuInd(stuName)) + ".stuans");
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                xml.writeTextElement("StuAns", file.readAll());
                file.close();
            }
        }

        xml.writeEndElement();
        xml.writeEndDocument();
        qint64 ret = tcpSendDatagram(client, array);
        log(client, QString("传输试卷 长度:%1 发送:%2").arg(array.length() + 4).arg(ret));
    } else if(type == "AnsProc") {
        if(QDateTime::currentDateTime() < mDateTimeEnd) {
            QString strProc = root.text();
            mConfigFile.setValue(QString("Stu/%1_Proc").arg(stuInd(stuName)), strProc);
            setStuProc(stuName, strProc.toInt());
        }
    } else if(type == "StuAns") {
        if(QDateTime::currentDateTime() < mDateTimeEnd) {
            int ind = stuInd(stuName);
            QFile file(mDirPath + "/" + QString::number(ind) + ".stuans");
            if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(array);
                file.close();
                setStuUploadTime(stuName, QDateTime::fromString(root.attribute("Time"), "yyyy/M/d H:m:s"));
                log(client, "\"" + stuName + "\" 上传作答 成功");

                QByteArray array;
                QXmlStreamWriter xml(&array);
                xml.writeStartDocument();
                xml.writeStartElement("ESDtg");
                xml.writeAttribute("Type", "StuAnsReceived");
                xml.writeCharacters(root.attribute("Time"));
                xml.writeEndElement();
                xml.writeEndDocument();
                tcpSendDatagram(client, array);
            } else log(client, "\"" + stuName + "\" 上传作答 失败");
        }
    } else if(type == "StuFinish") {
        if(QDateTime::currentDateTime() < mDateTimeEnd) {
            log(client, "\"" + stuName + "\" 交卷");
            bool ok;
            ScoreResult sr = score(stuName, &ok);
            if(ok) {
                int ind = stuInd(stuName);
                // 将改分信息写入文件
                QFile fileScore(mDirPath + "/" + QString::number(ind) + ".stuscore");
                if(fileScore.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QXmlStreamWriter xml(&fileScore);
                    xml.setAutoFormatting(true);
                    xml.writeStartDocument();
                    writeScoreResultToXml(stuName, sr, xml);
                    xml.writeEndDocument();
                    fileScore.close();
                } else log("写入 \"" + stuName + "\" 得分文件失败");

                // 发送改分信息
                QByteArray array;
                QXmlStreamWriter xml(&array);
                xml.writeStartDocument();
                xml.writeStartElement("ESDtg");
                xml.writeAttribute("Type", "StuFinishRetval");
                if(ui->cbbScoreInClient->isChecked())
                    writeScoreResultToXml(stuName, sr, xml);
                xml.writeEndElement();
                xml.writeEndDocument();
                tcpSendDatagram(client, array);
            }
        }
    } else return false;

    return true;
}

qint64 ExamWidget::tcpSendVerifyErr(QTcpSocket *client, const QString &what) {
    QByteArray array;
    QXmlStreamWriter xml(&array);
    xml.writeStartDocument();
    xml.writeStartElement("ESDtg");
    xml.writeAttribute("Type", "VerifyErr");
    xml.writeCharacters(what);
    xml.writeEndElement();
    xml.writeEndDocument();
    return tcpSendDatagram(client, array);
}

qint64 ExamWidget::tcpSendDatagram(QTcpSocket *client, const QByteArray &array) {
    int len = array.length();
    return client->write(QByteArray((char*)&len, 4) + array);
}

void ExamWidget::onUdpReadyRead() {
    while(mUdpSocket->hasPendingDatagrams()) {
        parseUdpDatagram(mUdpSocket->receiveDatagram());
    }
//    QByteArray datagram;
//    while (mUdpSocket->hasPendingDatagrams()) {
//        datagram.resize(int(mUdpSocket->pendingDatagramSize()));
//        mUdpSocket->readDatagram(datagram.data(), datagram.size());
//        parseUdpDatagram(datagram);
//    }
}

void ExamWidget::onNewConnection() {
    QTcpSocket *client = mTcpServer->nextPendingConnection();

    // 当考试已结束时，停止连入
    if(QDateTime::currentDateTime() >= mDateTimeEnd) {
        tcpSendVerifyErr(client, "考试已结束");
        client->flush();
        client->disconnectFromHost();
        return;
    }

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

                stuName = root.attribute("StuName");
                // 判断考生是否存在
                const Stu *stu = findStu(stuName);
                if(!stu) {
                    tcpSendVerifyErr(client, "考生不存在");
                    client->flush();
                    break;
                }
                // 当被占用时，拒绝连入
                bool isMulti = false;
                for(const Client &stuClient : mMapStuClient) {
                    if(stuClient.stuName == stuName) {
                        tcpSendVerifyErr(client, "考生已在考试中");
                        client->flush();
                        isMulti = true;
                        break;
                    }
                }
                if(isMulti)
                    break;

                // 验证密码
                uint salt = root.attribute("Salt").toUInt();
                QByteArray verify;
                QDataStream ds(&verify, QIODevice::WriteOnly);
                ds << client->peerAddress().toIPv4Address() << client->peerPort() << stu->pwd << salt;
                if(QCryptographicHash::hash(verify, QCryptographicHash::Md5).toHex() != root.text()) {
                    tcpSendVerifyErr(client, "密码错误");
                    client->flush();
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

    // 如果为交卷后进入，则删除原有作答
    int ind = stuInd(stuName);
    if(mConfigFile.value(QString("Stu/%1_Scored").arg(ind), false).toBool()) {
        QFile file(mDirPath + "/" + QString::number(ind) + ".stuans");
        file.remove();
    }
    mConfigFile.setValue(QString("Stu/%1_Scored").arg(ind), false);

    // 设置相关内容
    mMapStuClient[client] = stuName;
    setStuIsConnected(stuName, true);
    connect(client, &QTcpSocket::readyRead, this, &ExamWidget::onTcpReadyRead);
    connect(client, &QTcpSocket::disconnected, this, [this, client, stuName] {
        mMapStuClient.remove(client);
        setStuIsConnected(stuName, false);
        log(client, "\"" + stuName + "\" 断开连接");
    });
}

//#define DEBUG_TCP

void ExamWidget::onTcpReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    Client &c = mMapStuClient[client];
    c.buffer += client->readAll();

#ifdef DEBUG_TCP
    qDebug().noquote() << "Buffer: " << c.buffer.size();
#endif

    if(c.buffer.length() < 4)
        return;
    int len = *reinterpret_cast<int*>(c.buffer.data());

#ifdef DEBUG_TCP
    qDebug().noquote() << "Len: " << len;
#endif

    while(c.buffer.length() >= 4 + len) {
#ifdef DEBUG_TCP
        qDebug().noquote() << "Parsed: " << len;
#endif

        parseTcpDatagram(client, c.buffer.mid(4, len));
        c.buffer.remove(0, 4 + len);

        if(c.buffer.length() < 4)
            break;
        len = *reinterpret_cast<int*>(c.buffer.data());
    }
}

void ExamWidget::closeEvent(QCloseEvent *ev) {
    int ret = QMessageBox::information(this, "提示",
                                       "确认要关闭吗?\n"
                                       "将会断开考试有关连接\n"
                                       "你可以在菜单栏的 考试->历次考试 中再次打开该考试",
                                       "确认", "取消");
    if(ret == 1) {
        ev->ignore();
    }
}

const ExamWidget::Stu* ExamWidget::findStu(const QString &name) {
    for(const Stu &stu : mListStu) {
        if(stu.name == name)
            return &stu;
    }
    return nullptr;
}

