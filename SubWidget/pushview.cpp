#include "pushview.h"
#include "ui_pushview.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QTextStream>
#include <QXmlStreamWriter>

#include <QDateTime>
#include <QCryptographicHash>

#include "Util/header.h"
#include "Util/config.h"
#include "editview.h"

PushView::PushView(EditView *editView, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PushView), mEditView(editView)
{
    ui->setupUi(this);

    // 初始化表头
    const QStringList horHeaderText = {
        "序号", "考生"
    };
    ui->tableWidget->setColumnCount(horHeaderText.size());
    int i = 0;
    for(const QString &text : horHeaderText) {
        ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(text));
        i++;
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    // 绑定按钮信号与槽
    connect(ui->btnImport, &QPushButton::clicked, this, &PushView::onImport);
    connect(ui->btnBack, SIGNAL(clicked()), this, SIGNAL(back()));
    connect(ui->btnPush, &QPushButton::clicked, this, &PushView::onPush);
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

void PushView::setProjName(const QString &projName) {
    ui->labelProjName->setText(projName);
}

void PushView::writeAttributesXml(QXmlStreamWriter &xml) {
    const QString dateTimeFmt = "yyyy/M/d H:m:s";
    xml.writeAttribute("Name", ui->labelProjName->text());
    xml.writeAttribute("StartDateTime", ui->dateTimeEditStart->dateTime().toString(dateTimeFmt));
    xml.writeAttribute("EndDateTime", ui->dateTimeEditEnd->dateTime().toString(dateTimeFmt));
    xml.writeAttribute("ScoreInClient", QString::number(ui->cbbScoreInClient->isChecked()));
}
void PushView::writeStuListXml(QXmlStreamWriter &xml) {
    xml.writeStartElement("StuList");
    for(const Stu &stu : mListStu) {
        xml.writeStartElement("v");
        xml.writeAttribute("Pwd", stu.pwd);
        xml.writeCharacters(stu.name);
        xml.writeEndElement();
    }
    xml.writeEndElement();
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

    // 读取文件的每一行
    while(!file.atEnd()) {
        QStringList list = QString(file.readLine()).split('|');
        if(list.size() < 2)
            continue;
        QString name = list[0].trimmed();
        if(listStuContains(name))
            continue;
        mListStu << Stu{ name, list[1].trimmed() };
    }
    file.close();

    ui->tableWidget->setRowCount(mListStu.size());

    // 添加表格物件
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

void PushView::onPush() {
    // 检查列表是否为空
    if(mListStu.isEmpty()) {
        QMessageBox::warning(this, "错误", "考生列表为空");
        return;
    }

    // 检查时间
    QDateTime currentTime = QDateTime::currentDateTime();
    if(ui->dateTimeEditEnd->dateTime() < ui->dateTimeEditStart->dateTime()) {
        QMessageBox::warning(this, "错误", "结束时间不得早于开始时间");
        return;
    }
    if(ui->dateTimeEditEnd->dateTime() < currentTime) {
        QMessageBox::warning(this, "错误", "结束时间不得早于当前时间");
        return;
    }

    // 目录名
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(QByteArray::number(currentTime.date().toJulianDay()));
    hash.addData(QByteArray::number(currentTime.time().msecsSinceStartOfDay()));
    hash.addData(ui->labelProjName->text().toUtf8());
    QString dirName(hash.result().toHex());

    // 创建目录
    QDir dir(APP_DIR);
    QString path = "Exported/" + dirName;
    dir.mkpath(path);
    if(!dir.cd(path)) {
        QMessageBox::critical(this, "错误", "操作失败");
        return;
    }

    // 写入 _.esep
    QFile fileExported(dir.absoluteFilePath("_.esep"));
    if(!fileExported.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "操作失败");
        return;
    }
    // 使用XML写入
    QXmlStreamWriter xml(&fileExported);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("ExamSysExportedProject");
    writeAttributesXml(xml);
    writeStuListXml(xml);
    mEditView->writeQuesXml(xml);
    xml.writeEndElement();
    xml.writeEndDocument();

    fileExported.close();

    emit exam(dirName);
}
