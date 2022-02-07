#include "editview.h"
#include "ui_editview.h"

#include <QMenu>

#include "Ques/quessinglechoice.h"
#include "ui_addquesdialog.h"

EditView::EditView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditView),
    availableQues({
                  {"QuesSingleChoice", {"单选题", &QuesSingleChoice::staticMetaObject}}
                  })
{
    ui->setupUi(this);

    ui->splitter->setSizes(QList<int>() << 300 << 100);
    ui->splitterRight->setSizes(QList<int>() << 300 << 100);
    ui->listWidget->setAcceptDrops(true);
    ui->listWidget->setDragEnabled(true);
    ui->listWidget->setDragDropMode(QListWidget::DragDropMode::InternalMove);
    connect(ui->listWidget, &DropSignalListWidget::drop, this, &EditView::updateIndex);
    connect(ui->listWidget, &DropSignalListWidget::itemDoubleClicked, this, &EditView::onItemDoubleClicked);

    connect(ui->btnAdd, &QPushButton::clicked, this, &EditView::onAddClicked);
}

EditView::~EditView()
{
    delete ui;
}

void EditView::createQues(const QMetaObject *pMetaObject) {
    Ques *ques = (Ques*)pMetaObject->newInstance();
    if(!ques)
        return;
    ques->adjustSize();
    ques->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(ques, &Ques::customContextMenuRequested, this, &EditView::onCustomContextMenuRequested);

    QListWidgetItem *item = new QListWidgetItem;
    //item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    item->setSizeHint(ques->size());

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, ques);
    ques->setNumber(ui->listWidget->count());
}

void EditView::onItemDoubleClicked(QListWidgetItem *item) {
    Ques *ques = (Ques*)ui->listWidget->itemWidget(item);
    Ques *other = ques->edit();
    if(other) {
        ui->listWidget->removeItemWidget(item);
        ques->deleteLater();

        other->setNumber(ui->listWidget->row(item) + 1);
        ui->listWidget->setItemWidget(item, other);
        item->setSizeHint(other->sizeHint());
        other->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
        connect(other, &Ques::customContextMenuRequested, this, &EditView::onCustomContextMenuRequested);
    }
}

void EditView::onCustomContextMenuRequested(const QPoint &) {
    Ques *ques = qobject_cast<Ques*>(sender());
    auto getItem = [this, ques]() -> QListWidgetItem* {
        int count = ui->listWidget->count();
        for(int i = 0; i < count; i++) {
            QListWidgetItem *item = ui->listWidget->item(i);
            if(ui->listWidget->itemWidget(item) == ques)
                return item;
        }
        return nullptr;
    };

    QMenu menu;

    QAction actDelete("删除");
    menu.addAction(&actDelete);
    connect(&actDelete, &QAction::triggered, [this, ques, getItem] {
        QListWidgetItem *item = getItem();
        if(!item)
            return;
        ui->listWidget->removeItemWidget(item);
        ques->deleteLater();
        delete ui->listWidget->takeItem(ui->listWidget->row(item));

        updateIndex();
    });

    menu.move(cursor().pos());
    menu.exec();
}

void EditView::updateIndex() {
    int count = ui->listWidget->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        Ques *ques = (Ques*)ui->listWidget->itemWidget(item);
        ques->setNumber(i + 1);
    }
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
