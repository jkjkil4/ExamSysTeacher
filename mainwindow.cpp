#include "mainwindow.h"
#include "ui_mainwindow.h"

// 控件相关
#include <QStackedLayout>
#include <QMessageBox>
#include <QCloseEvent>

// 文件相关
#include <QFileDialog>
#include <QFileInfo>

// XML相关
#include <QXmlStreamWriter>
#include <QDomDocument>

// 自写
#include "SubWidget/mainview.h"
#include "SubWidget/editview.h"
#include "SubWidget/pushview.h"
#include "Util/config.h"
#include "Util/header.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      mStkLayout(new QStackedLayout),
      mMainView(new MainView(this)), mEditView(new EditView(this)), mPushView(new PushView(this))
{
    ui->setupUi(this);

    mPushView->autoAdjustDateTime();

    mStkLayout->setMargin(0);
    mStkLayout->addWidget(mMainView);
    mStkLayout->addWidget(mEditView);
    mStkLayout->addWidget(mPushView);
    mStkLayout->setCurrentWidget(mMainView);

    ui->centralwidget->setLayout(mStkLayout);

    setWindowTitle("考试系统（教师端）");
    resize(1240, 760);

    connect(ui->actNewProj, &QAction::triggered, this, &MainWindow::onNewProj);
    connect(ui->actLoadProj, &QAction::triggered, this, &MainWindow::onLoadProj);
    connect(ui->actSaveProj, &QAction::triggered, this, &MainWindow::onSaveProj);
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actAboutQt, &QAction::triggered, this, &MainWindow::onAboutQt);

    connect(mMainView, &MainView::newProjClicked, this, &MainWindow::onNewProj);
    connect(mMainView, &MainView::loadProjClicked, this, &MainWindow::onLoadProj);

    connect(mEditView, &EditView::changed, [this] { mIsChanged = true; });
    connect(mEditView, &EditView::push, this, &MainWindow::onPush);

    connect(mPushView, &PushView::back, this, &MainWindow::onPushViewBack);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::newProj(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "新建失败");
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.writeStartDocument();
    xml.writeStartElement("ExamSysProject");
    xml.writeAttribute("Version", QString::number(PROJ_VERSION));
    xml.writeEndElement();
    xml.writeEndDocument();

    file.close();

    mEditView->clear();

    return true;
}
bool MainWindow::loadProj(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "加载失败，文件无法打开");
        return false;
    }

    QDomDocument doc;
    doc.setContent(&file);
    if(doc.isNull()) {
        QMessageBox::critical(this, "错误", "加载失败，文件无效");
        return false;
    }

    QDomElement root = doc.documentElement();
    if(root.tagName() != "ExamSysProject") {
        QMessageBox::critical(this, "错误", "加载失败，非试卷文件");
        return false;
    }

    QDomNode node = root.firstChild();
    QDomElement elemQuesList;
    QDomElement elemQuesConf;
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        if(elem.tagName() == "QuesList") {
            elemQuesList = elem;
        } else if(elem.tagName() == "QuesConf") {
            elemQuesConf = elem;
        }
        node = node.nextSibling();
    }
    mEditView->readQuesXml(elemQuesList);
    mEditView->readConfXml(elemQuesConf);

    return true;
}
bool MainWindow::saveProj(const QString &filePath) {
    QFile file(filePath);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "保存失败");
        return false;
    }

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("ExamSysProject");
    xml.writeAttribute("Version", QString::number(PROJ_VERSION));
    mEditView->writeQuesXml(xml);
    mEditView->writeConfXml(xml);
    xml.writeEndElement();
    xml.writeEndDocument();

    file.close();

    return true;
}

bool MainWindow::verifyClose() {
    int ret = QMessageBox::information(this, "提示", "文件未保存", "保存", "不保存", "取消");
    switch(ret) {
    case 0:
        mIsChanged = false;
        return saveProj(mProjPath);
    case 1:
        mIsChanged = false;
        return true;
    default:
        return false;
    }
}

void MainWindow::onNewProj() {
    QWidget *currentWidget = mStkLayout->currentWidget();
    if(currentWidget != mMainView && currentWidget != mEditView)
        return;

    if(mIsChanged && !verifyClose())
        return;

    Config config;
    QString filePath = QFileDialog::getSaveFileName(
                this, "新试卷路径", config.value("EST/SaveExamPath").toString() + "/untitled.estp",
                "试卷文件 (*.estp);;所有文件(*.*)");
    if(filePath.isEmpty())
        return;
    config.setValue("EST/SaveExamPath", QFileInfo(filePath).path());

    if(!newProj(filePath))
        return;
    mProjPath = filePath;
    mEditView->setProjName(QFileInfo(mProjPath).completeBaseName());
    mStkLayout->setCurrentWidget(mEditView);
}
void MainWindow::onLoadProj() {
    QWidget *currentWidget = mStkLayout->currentWidget();
    if(currentWidget != mMainView && currentWidget != mEditView)
        return;

    if(mIsChanged && !verifyClose())
        return;

    Config config;
    QString filePath = QFileDialog::getOpenFileName(
                this, "读取试卷", config.value("EST/LoadExamPath").toString(),
                "试卷文件 (*.estp);;所有文件(*.*)");
    if(filePath.isEmpty())
        return;
    config.setValue("EST/LoadExamPath", QFileInfo(filePath).path());

    if(!loadProj(filePath))
        return;
    mProjPath = filePath;
    mEditView->setProjName(QFileInfo(mProjPath).completeBaseName());
    mStkLayout->setCurrentWidget(mEditView);
}
void MainWindow::onSaveProj() {
    QWidget *currentWidget = mStkLayout->currentWidget();
    if(currentWidget != mEditView)
        return;

    if(saveProj(mProjPath))
        mIsChanged = false;
}

void MainWindow::onPush() {
    if(!saveProj(mProjPath))
        return;
    mIsChanged = false;

    ui->actNewProj->setEnabled(false);
    ui->actLoadProj->setEnabled(false);
    ui->actSaveProj->setEnabled(false);
    mStkLayout->setCurrentWidget(mPushView);
}
void MainWindow::onPushViewBack() {
    ui->actNewProj->setEnabled(true);
    ui->actLoadProj->setEnabled(true);
    ui->actSaveProj->setEnabled(true);
    mStkLayout->setCurrentWidget(mProjPath.isEmpty() ? (QWidget*)mMainView : mEditView);
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

void MainWindow::closeEvent(QCloseEvent *ev) {
    if(mIsChanged && !verifyClose())
        ev->ignore();
}
