#include "editview.h"
#include "ui_editview.h"

#include "Ques/quessinglechoice.h"

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

    createQues(&QuesSingleChoice::staticMetaObject);
    createQues(&QuesSingleChoice::staticMetaObject);
    createQues(&QuesSingleChoice::staticMetaObject);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &EditView::onItemDoubleClicked);
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

    QListWidgetItem *item = new QListWidgetItem;
    //item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    item->setSizeHint(ques->sizeHint());

    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, ques);
    ques->setNumber(ui->listWidget->count());
}

void EditView::onItemDoubleClicked(QListWidgetItem *item) {
    Ques *ques = (Ques*)ui->listWidget->itemWidget(item);
    if(ques->edit()) {
        int width = ques->width();
        ques->adjustSize();
        ques->resize(width, ques->height());
        item->setSizeHint(ques->sizeHint());
    }
}

void EditView::updateIndex() {
    int count = ui->listWidget->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem *item = ui->listWidget->item(i);
        Ques *ques = (Ques*)ui->listWidget->itemWidget(item);
        ques->setNumber(i + 1);
    }
}
