#include "quessinglechoice.h"
#include "ui_quessinglechoiceeditdialog.h"

#include <QLabel>
#include <QRadioButton>

#include <QVBoxLayout>
#include <QRandomGenerator>

#include <QXmlStreamWriter>
#include <QDomDocument>

QuesSingleChoice::QuesSingleChoice(QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel("（单选题）")), mLayout(new QVBoxLayout), mLayoutButtons(new QVBoxLayout)
{
    QFont font = mLabelQues->font();
    font.setPointSize(10);
    mLabelQues->setFont(font);
    mLabelQues->setWordWrap(true);

    mLayout->addWidget(mLabelQues);
    mLayout->addSpacing(16);
    mLayout->addLayout(mLayoutButtons);
    mFrame->setLayout(mLayout);
}

bool QuesSingleChoice::edit() {
    QDialog dialog;
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
        list.clear();
        int btnCnt = mLayoutButtons->count();
        for(int i = 0; i < btnCnt; i++) {
            mLayoutButtons->itemAt(i)->widget()->deleteLater();
        }

        int count = ui.listWidget->count();
        for(int i = 0; i < count; ++i) {
            QString itemText = ui.listWidget->item(i)->text();

            list << itemText;

            QString str;
            int tmp = i;
            do {
                str += 'A' + tmp % 26;
                tmp /= 26;
            } while(tmp);
            str += ". " + itemText;
            QRadioButton *rb = new QRadioButton(str);
            mLayoutButtons->addWidget(rb);
            connect(rb, &QRadioButton::clicked, this, [this, i](bool checked) {
                if(checked)
                    trueAns = i;
            });
        }

        text = ui.editQues->toPlainText();
        mLabelQues->setText("（单选题）" + text);
        return true;
    }
    return false;
}

void QuesSingleChoice::writeXml(QXmlStreamWriter &xml) const {
    xml.writeStartElement("QuesSingleChoice");
    xml.writeTextElement("Ques", text);
    int trueAns = -1;
    for(const QString &str : list) {
        xml.writeTextElement("Ans", str);
    }
    xml.writeTextElement("TrueAns", QString::number(trueAns));
    xml.writeEndElement();
}
void QuesSingleChoice::readXml(const QDomElement &elem) {
    Q_UNUSED(elem)
    // TODO: ...
}
