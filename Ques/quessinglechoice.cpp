#include "quessinglechoice.h"
#include "ui_quessinglechoiceeditdialog.h"

#include <QLabel>

#include <QVBoxLayout>
#include <QRandomGenerator>

QuesSingleChoice::QuesSingleChoice(QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel("（单选题）")), mLabelList(new QLabel)
{
    QFont font = mLabelQues->font();
    font.setPointSize(10);
    mLabelQues->setFont(font);
    mLabelQues->setWordWrap(true);
    mLabelList->setFont(font);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabelQues);
    layout->addWidget(mLabelList);
    mFrame->setLayout(layout);
}

bool QuesSingleChoice::edit() {
    QDialog dialog(this);
    Ui::QuesSingleChoiceEditDialog ui;
    ui.setupUi(&dialog);
    ui.editQues->setPlainText(text);
    for(const QString &str : list) {
        QListWidgetItem *item = new QListWidgetItem(str);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.listWidget->addItem(item);
    }

    // 添加选项
    connect(ui.btnAdd, &QPushButton::clicked, [&ui] {
        QListWidgetItem *item = new QListWidgetItem("选项");
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui.listWidget->addItem(item);
    });
    // 删除选项
    connect(ui.btnRemove, &QPushButton::clicked, [&ui] {
        QList<QListWidgetItem*> items = ui.listWidget->selectedItems();
        for(QListWidgetItem *item : qAsConst(items)) {
            ui.listWidget->takeItem(ui.listWidget->row(item));
            delete item;
        }
    });
    // 打乱
    connect(ui.btnUpset, &QPushButton::clicked, [&ui] {
        QStringList list;
        int count = ui.listWidget->count();
        for(int i = 0; i < count; ++i) {
            list << ui.listWidget->item(i)->text();
        }
        ui.listWidget->clear();

        QRandomGenerator *rand = QRandomGenerator::global();
        for(uint i = (uint)list.size(); i > 0; --i) {
            int ind = (int)(rand->generate() % i);
            QString text = list.takeAt(ind);
            QListWidgetItem *item = new QListWidgetItem(text);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui.listWidget->addItem(item);
        }
    });

    if(dialog.exec()) {
        text = ui.editQues->toPlainText();
        list.clear();
        QString str;
        int count = ui.listWidget->count();
        for(int i = 0; i < count; ++i) {
            QString itemText = ui.listWidget->item(i)->text();
            list << itemText;
            str += '\n';
            int tmp = i;
            do {
                str += 'A' + tmp % 26;
                tmp /= 26;
            } while(tmp);
            str += ". " + itemText;
        }

        mLabelQues->setText("（单选题）" + text);
        mLabelList->setText(str);
        return true;
    }
    return false;
}

void QuesSingleChoice::writeXml(QXmlStreamWriter &xml) {
    Q_UNUSED(xml)
    // TODO: ...
}
void QuesSingleChoice::readXml(const QDomElement &elem) {
    Q_UNUSED(elem)
    // TODO: ...
}
