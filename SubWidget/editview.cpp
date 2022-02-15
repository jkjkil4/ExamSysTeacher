#include "editview.h"
#include "ui_editview.h"

#include <QMenu>

#include <QXmlStreamWriter>
#include <QDomElement>

#include "Ques/quessinglechoice.h"
#include "Ques/quesmultichoice.h"
#include "Ques/queswhether.h"
#include "ui_addquesdialog.h"

EditView::EditView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditView), mLayoutScrollItems(new QVBoxLayout),
    availableQues({
                  {"QuesSingleChoice", "单选题", &QuesSingleChoice::staticMetaObject},
                  {"QuesMultiChoice", "多选题", &QuesMultiChoice::staticMetaObject},
                  {"QuesWhether", "判断题", &QuesWhether::staticMetaObject}
                  })
{
    ui->setupUi(this);

    // 配置splitter大小比例
    ui->splitter->setSizes(QList<int>() << 400 << 100);
    ui->splitterRight->setSizes(QList<int>() << 300 << 100 << 100);

    // 绑定按钮信号与槽
    connect(ui->btnAdd, &QPushButton::clicked, this, &EditView::onAddClicked);
    connect(ui->btnPush, &QPushButton::clicked, this, &EditView::onPushClicked);

    // 列表布局
    QVBoxLayout *layoutScroll = new QVBoxLayout;
    layoutScroll->addLayout(mLayoutScrollItems);
    layoutScroll->addStretch();
    ui->scrollArea->widget()->setLayout(layoutScroll);
    ui->scrollArea->widget()->setObjectName("ScrollAreaWidget");
    ui->scrollArea->widget()->setStyleSheet("QWidget#ScrollAreaWidget{background-color: white;}");
}

EditView::~EditView()
{
    delete ui;
}

Ques* EditView::createQues(const QMetaObject *pMetaObject) {
    Ques *ques = (Ques*)pMetaObject->newInstance();
    if(!ques)
        return nullptr;

    // 设置自定义右键菜单
    ques->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    // 绑定信号与槽
    connect(ques, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(ques, &Ques::doubleClicked, this, &EditView::onDoubleClicked);
    connect(ques, &Ques::customContextMenuRequested, this, &EditView::onCustomContextMenuRequested);

    // 添加控件
    mLayoutScrollItems->addWidget(ques);
    updateIndex(mLayoutScrollItems->count() - 1);

    updateInfo();

    return ques;
}

void EditView::updateIndex(int ind, int val) {
    Ques *ques = (Ques*)mLayoutScrollItems->itemAt(ind)->widget();
    ques->setNumber((val == -1 ? ind : val) + 1);
}

void EditView::writeQuesXml(QXmlStreamWriter &xml) {
    xml.writeStartElement("QuesList");
    // 遍历所有题目控件，将题目写入XML
    int count = mLayoutScrollItems->count();
    for(int i = 0; i < count; ++i) {
        Ques *ques = (Ques*)mLayoutScrollItems->itemAt(i)->widget();
        ques->writeXml(xml);
    }
    xml.writeEndElement();
}
void EditView::readQuesXml(const QDomElement &elem) {
    clearQues();
    blockSignals(true);
    // 遍历所有子节点
    QDomNode node = elem.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        const QuesType *p = findQues(elem.tagName());
        if(p) {
            Ques *ques = createQues(p->pMetaObject);
            if(ques) {
                ques->readXml(elem);
            }
        }
        node = node.nextSibling();
    }
    blockSignals(false);

    updateInfo();
}

void EditView::writeConfXml(QXmlStreamWriter &xml) {
    Q_UNUSED(xml)
    // TODO: ...
}
void EditView::readConfXml(const QDomElement &elem) {
    Q_UNUSED(elem)
    // TODO: ...
}

void EditView::updateInfo() {
    QString info;
    info += "名称: " + mProjName + '\n';
    info += "题目量: " + QString::number(mLayoutScrollItems->count()) + '\n';
    ui->labelInfoText->setText(info);
}

void EditView::clearQues() {
    int count = mLayoutScrollItems->count();
    for(int i = 0; i < count; ++i) {
        QLayoutItem *item = mLayoutScrollItems->takeAt(0);
        item->widget()->deleteLater();
        delete item;
    }
}
void EditView::clear() {
    clearQues();
    updateInfo();
}

void EditView::onDoubleClicked() {
    Ques *ques = qobject_cast<Ques*>(sender());
    if(ques->edit())
        emit changed();
}

void EditView::onCustomContextMenuRequested(const QPoint &) {
    Ques *ques = qobject_cast<Ques*>(sender());

    QMenu menu;

    QAction actMoveUp("上移");
    actMoveUp.setEnabled(mLayoutScrollItems->indexOf(ques) != 0);
    menu.addAction(&actMoveUp);
    connect(&actMoveUp, &QAction::triggered, [this, ques] {
        int ind = mLayoutScrollItems->indexOf(ques);
        if(ind == 0)
            return;
        mLayoutScrollItems->insertItem(ind, mLayoutScrollItems->takeAt(ind - 1));
        updateIndex(ind - 1);
        updateIndex(ind);

        emit changed();
    });

    QAction actMoveDown("下移");
    actMoveDown.setEnabled(mLayoutScrollItems->indexOf(ques) != mLayoutScrollItems->count() - 1);
    menu.addAction(&actMoveDown);
    connect(&actMoveDown, &QAction::triggered, [this, ques] {
        int ind = mLayoutScrollItems->indexOf(ques);
        if(ind == mLayoutScrollItems->count() - 1)
            return;
        mLayoutScrollItems->insertItem(ind, mLayoutScrollItems->takeAt(ind + 1));
        updateIndex(ind + 1);
        updateIndex(ind);

        emit changed();
    });

    menu.addSeparator();

    QAction actDelete("删除");
    menu.addAction(&actDelete);
    connect(&actDelete, &QAction::triggered, [this, ques] {
        int ind = mLayoutScrollItems->indexOf(ques);
        int count = mLayoutScrollItems->count();
        for(int i = ind + 1; i < count; ++i) {
            updateIndex(i, i - 1);
        }
        QLayoutItem *item = mLayoutScrollItems->takeAt(ind);
        delete item->widget();
        delete item;

        updateInfo();

        emit changed();
    });

    menu.move(cursor().pos());
    menu.exec();
}

void EditView::onAddClicked() {
    QDialog dialog(this);
    Ui::AddQuesDialog ui;
    ui.setupUi(&dialog);

    // 将所有题目类型添加到cbbTypes中
    for(const QuesType &qtype : availableQues) {
        ui.cbbTypes->addItem(qtype.name);
    }

    if(dialog.exec()) {
        createQues(availableQues[ui.cbbTypes->currentIndex()].pMetaObject);
        emit changed();
    }
}
void EditView::onPushClicked() {
    struct Unfinished { int ind; QString what; };

    // 遍历题目检查是否有未完成的题目
    QList<Unfinished> listUnfinished;
    int count = mLayoutScrollItems->count();
    for(int i = 0; i < count; ++i) {
        Ques *ques = (Ques*)mLayoutScrollItems->itemAt(i)->widget();
        QString what = ques->isDone();
        if(!what.isEmpty()) {
            listUnfinished << Unfinished{ i, what };
        }
    }

    // 如果没有未完成的题目，则发出push()信号并退出函数
    if(listUnfinished.isEmpty()) {
        emit push();
        return;
    }

    // 创建控件并显示
    QVBoxLayout *layoutArea = new QVBoxLayout;
    for(const Unfinished &unf : listUnfinished) {
        QLabel *label = new QLabel(QString::number(unf.ind + 1) + '.');
        label->setObjectName("left");
        QLabel *labelWhat = new QLabel(unf.what);
        labelWhat->adjustSize();
        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(label);
        layout->addWidget(labelWhat, 1);
        layoutArea->addLayout(layout);
    }
    layoutArea->addStretch();

    QScrollArea *area = new QScrollArea;
    area->setStyleSheet("QScrollArea{"
                        "    background-color: white;"
                        "}"
                        "QLabel#left{"
                        "    border: 2px solid rgb(26, 222, 209);"
                        "    color: #FFFFFF;"
                        "    background-color: rgb(26, 222, 209);"
                        "}");
    QWidget *areaWidget = new QWidget;
    areaWidget->setObjectName("areaWidget");
    areaWidget->setStyleSheet("QWidget#areaWidget{"
                              "    background-color: white;"
                              "}");
    areaWidget->setLayout(layoutArea);
    area->setWidget(areaWidget);

    QHBoxLayout *layoutDialog = new QHBoxLayout;
    layoutDialog->setMargin(0);
    layoutDialog->addWidget(area);

    QDialog dialog(this);
    dialog.setWindowTitle("以下题目未完成");
    dialog.setLayout(layoutDialog);
    dialog.resize(300, 400);
    dialog.exec();
}

void EditView::setProjName(const QString &projName) {
    if(projName != mProjName) {
        mProjName = projName;
        updateInfo();
    }
}

const EditView::QuesType *EditView::findQues(const QString &key) {
    for(const QuesType &qtype : availableQues) {
        if(qtype.key == key)
            return &qtype;
    }
    return nullptr;
}
