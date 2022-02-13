#include "examwidget.h"
#include "ui_examwidget.h"

#include <QDomDocument>
#include <QTextStream>

#include "Util/header.h"

ExamWidget::ExamWidget(const QString &dirName, QWidget *parent)
    : QWidget(parent), ui(new Ui::ExamWidget),
      mDirName(dirName), mDirPath(APP_DIR + "/Exported/" + dirName)
{
    ui->setupUi(this);
    setEnabled(false);

    // 读取信息
    QFile fileExported(mDirPath + "/exported.esep");
    if(fileExported.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc;
        do {
            if(!doc.setContent(&fileExported))
                break;
            QDomElement root = doc.documentElement();
            if(root.tagName() != "ExamSysExportedProject")
                break;
            QDomNode nodeQuesList = root.elementsByTagName("QuesList").item(0);
            if(nodeQuesList.isNull())
                break;
            mName = root.attribute("Name");
            mDateTimeStart = QDateTime::fromString(root.attribute("StartDateTime"), "yyyy/M/d H:m:s");
            mDateTimeEnd = QDateTime::fromString(root.attribute("EndDateTime"), "yyyy/M/d H:m:s");
            mScoreInClient = root.attribute("ScoreInClient").toInt();
            mQuesCnt = nodeQuesList.childNodes().count();
            mIsVaild = true;
        } while(false);
        fileExported.close();
    }

    if(!mIsVaild)
        return;

    // 读取考生列表
    mIsVaild = false;
    QFile fileStuList(mDirPath + "/exported.sl");
    if(fileStuList.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileStuList);
        while(!in.atEnd()) {
            QStringList list = in.readLine().split('|');
            if(list.size() < 2)
                continue;
            mListStu.append(Stu{ list[0], list[1] });
        }
        mIsVaild = true;
        fileStuList.close();
    }

    if(!mIsVaild)
        return;

    // 配置控件
    setEnabled(true);
    ui->labelExamName->setText(mName);
    ui->labelQuesCnt->setText(QString::number(mQuesCnt));
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
}
