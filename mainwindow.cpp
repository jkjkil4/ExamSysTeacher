#include "mainwindow.h"
#include "ui_mainwindow.h"

// 控件相关
#include <QStackedLayout>
#include <QMessageBox>

// 文件相关
#include <QFileDialog>
#include <QFileInfo>

// XML相关
#include <QXmlStreamWriter>

// 自写
#include "SubWidget/mainview.h"
#include "SubWidget/editview.h"
#include "Util/config.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      mStkLayout(new QStackedLayout), mMainView(new MainView(this)), mEditView(new EditView(this))
{
    ui->setupUi(this);

    mStkLayout->setMargin(0);
    mStkLayout->addWidget(mMainView);
    mStkLayout->addWidget(mEditView);
    mStkLayout->setCurrentWidget(mMainView);

    ui->centralwidget->setLayout(mStkLayout);

    setWindowTitle("考试系统（教师端）");
    resize(1100, 760);

    connect(ui->actNewProj, &QAction::triggered, this, &MainWindow::onNewProj);
    connect(ui->actLoadProj, &QAction::triggered, this, &MainWindow::onLoadProj);
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actAboutQt, &QAction::triggered, this, &MainWindow::onAboutQt);

    connect(mMainView, &MainView::newProjClicked, this, &MainWindow::onNewProj);
    connect(mMainView, &MainView::loadProjClicked, this, &MainWindow::onLoadProj);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newProj(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "新建失败");
        return;
    }

    QXmlStreamWriter xml(&file);
    xml.writeStartDocument();
    xml.writeStartElement("ExamSysProject");
    xml.writeEndElement();
    xml.writeEndDocument();

    file.close();
}
void MainWindow::loadProj(const QString &filePath) {
    Q_UNUSED(filePath)
    // TODO: ...
}

void MainWindow::onNewProj() {
    Config config;
    QString filePath = QFileDialog::getSaveFileName(
                this, "保存路径", config.value("EST/SaveExamPath").toString() + "/untitled.estp",
                "试卷文件 (*.estp);;所有文件(*.*)");
    if(filePath.isEmpty())
        return;
    config.setValue("EST/SaveExamPath", QFileInfo(filePath).path());
    newProj(filePath);
    mStkLayout->setCurrentWidget(mEditView);
}
void MainWindow::onLoadProj() {
    Config config;
    QString filePath = QFileDialog::getOpenFileName(
                this, "读取试卷", config.value("EST/LoadExamPath").toString(),
                "试卷文件 (*.estp);;所有文件(*.*)");
    if(filePath.isEmpty())
        return;
    config.setValue("EST/LoadExamPath", QFileInfo(filePath).path());
    loadProj(filePath);
}

void MainWindow::onAbout() {
    QMessageBox::about(
                this, "关于",
                "ExamSysTeacher v0.1<br>"
                "作者: jkjkil4<br>"
                "gitee: <a href=https://gitee.com/jkjkil4/ExamSysTeacher>https://gitee.com/jkjkil4/ExamSysTeacher</a>");
}
void MainWindow::onAboutQt() {
    QMessageBox::aboutQt(this);
}
