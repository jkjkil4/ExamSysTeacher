#include "editview.h"
#include "ui_editview.h"

#include <QMenu>

#include <QXmlStreamWriter>
#include <QDomElement>

#include "Ques/quessinglechoice.h"
#include "ui_addquesdialog.h"

EditView::EditView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditView), mLayoutScrollItems(new QVBoxLayout),
    availableQues({
                  {"QuesSingleChoice", {"单选题", &QuesSingleChoice::staticMetaObject}}
                  })
{
    ui->setupUi(this);

    ui->splitter->setSizes(QList<int>() << 400 << 100);
    ui->splitterRight->setSizes(QList<int>() << 300 << 100);

    connect(ui->btnAdd, &QPushButton::clicked, this, &EditView::onAddClicked);

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

Ques *EditView::createQues(const QMetaObject *pMetaObject) {
    Ques *ques = (Ques*)pMetaObject->newInstance();
    if(!ques)
        return nullptr;

    ques->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ques, &Ques::customContextMenuRequested, this, &EditView::onCustomContextMenuRequested);

    mLayoutScrollItems->addWidget(ques);
    updateIndex(mLayoutScrollItems->count() - 1);

    return ques;
}

void EditView::updateIndex(int ind, int val) {
    Ques *ques = (Ques*)mLayoutScrollItems->itemAt(ind)->widget();
    ques->setNumber((val == -1 ? ind : val) + 1);
}

void EditView::writeQuesXml(QXmlStreamWriter &xml) {
    xml.writeStartElement("QuesList");
    int count = mLayoutScrollItems->count();
    for(int i = 0; i < count; ++i) {
        Ques *ques = (Ques*)mLayoutScrollItems->itemAt(i)->widget();
        ques->writeXml(xml);
    }
    xml.writeEndElement();
}
void EditView::readQuesXml(const QDomElement &elem) {
    clearQues();
    QDomNode node = elem.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        auto iter = availableQues.constFind(elem.tagName());
        if(iter != availableQues.cend()) {
            Ques *ques = createQues(iter.value().pMetaObject);
            if(ques) {
                ques->readXml(elem);
            }
        }
        node = node.nextSibling();
    }
}

void EditView::clearQues() {
    int count = mLayoutScrollItems->count();
    for(int i = 0; i < count; ++i) {
        mLayoutScrollItems->takeAt(0)->widget()->deleteLater();
    }
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
        ques->deleteLater();
    });

    menu.move(cursor().pos());
    menu.exec();
}

void EditView::onAddClicked() {
    QDialog dialog(this);
    Ui::AddQuesDialog ui;
    ui.setupUi(&dialog);
    for(auto iter = availableQues.cbegin(); iter != availableQues.cend(); ++iter) {
        ui.cbbTypes->addItem(iter.value().name, iter.key());
    }
    if(dialog.exec()) {
        createQues(availableQues.value(ui.cbbTypes->currentData().toString()).pMetaObject);
    }
}
