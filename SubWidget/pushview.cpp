#include "pushview.h"
#include "ui_pushview.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QDateTime>

#include "Util/config.h"

PushView::PushView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PushView)
{
    ui->setupUi(this);

    const QStringList horHeaderText = {
        "序号", "考生", "连接情况", "答题进度", "最后一次提交时间", "成绩"
    };
    ui->tableWidget->setColumnCount(horHeaderText.size());
    int i = 0;
    for(const QString &text : horHeaderText) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(text));
        i++;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    connect(ui->btnImport, &QPushButton::clicked, this, &PushView::onImport);
    connect(ui->btnBack, SIGNAL(clicked()), this, SIGNAL(back()));
}

PushView::~PushView()
{
    delete ui;
}

void PushView::autoAdjustDateTime() {
    QDateTime dateTime = QDateTime::currentDateTime();
    QTime time = dateTime.time();
    time.setHMS(time.hour(), 0, 0);
    dateTime.setTime(time);
    ui->dateTimeEditStart->setDateTime(dateTime.addSecs(60 * 60));
    ui->dateTimeEditEnd->setDateTime(dateTime.addSecs(2 * 60 * 60));
}

bool PushView::listStuContains(const QString &name) {
    for(const Stu &stu : mListStu) {
        if(stu.name == name)
            return true;
    }
    return false;
}

void PushView::onImport() {
    Config config;
    QString filePath = QFileDialog::getOpenFileName(this, "导入考生列表", config.value("EST/ImportStuPath").toString());
    if(filePath.isEmpty())
        return;
    config.setValue("EST/ImportStuPath", QFileInfo(filePath).path());

    ui->tableWidget->clearContents();
    mListStu.clear();

    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "导入失败");
        return;
    }

    while(!file.atEnd()) {
        QStringList list = QString(file.readLine()).split('|');
        if(list.size() < 2)
            continue;
        QString name = list[0].trimmed();
        if(listStuContains(name))
            continue;
        mListStu << Stu{ name, list[1].trimmed() };
    }

    ui->tableWidget->setRowCount(mListStu.size());

    int i = 0;
    for(const Stu &stu : mListStu) {
        QTableWidgetItem *itemInd = new QTableWidgetItem(QString::number(i + 1));
        itemInd->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 0, itemInd);

        QTableWidgetItem *itemName = new QTableWidgetItem(stu.name);
        itemName->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui->tableWidget->setItem(i, 1, itemName);

        i++;
    }
}
