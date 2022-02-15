#include "examwidget.h"
#include "ui_examwidget.h"

#include <QDomDocument>
#include <QTextStream>

#include "Util/header.h"
#include "Ques/quessinglechoice.h"
#include "Ques/quesmultichoice.h"
#include "Ques/queswhether.h"

#include <QDebug>

ExamWidget::ExamWidget(const QString &dirName, bool hasEnd, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExamWidget),
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
                        qDebug() << data->metaObject()->className();
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
                    qDebug() << name << elem.attribute("Pwd");
                }
                node = node.nextSibling();
            }

            mIsVaild = true;
        } while(false);
    }

    if(!mIsVaild)
        return;

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

    setWindowTitle(mName);
    updateState();
}

void ExamWidget::updateState() {
    QDateTime currentTime = QDateTime::currentDateTime();
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

