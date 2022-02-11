#include "queschoice.h"
#include "ui_queschoiceeditdialog.h"

#include <QLabel>

#include <QVBoxLayout>
#include <QRandomGenerator>

#include <QXmlStreamWriter>
#include <QDomDocument>

QuesChoice::QuesChoice(const QString &head, QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel(head)), mLayout(new QVBoxLayout), mLayoutButtons(new QVBoxLayout),
      mHead(head)
{
    mLabelQues->setWordWrap(true);

    mLayout->addWidget(mLabelQues);
    mLayout->addSpacing(16);
    mLayout->addLayout(mLayoutButtons);
    mFrame->setLayout(mLayout);
}

bool QuesChoice::edit() {
    QDialog dialog(this);
    Ui::QuesChoiceEditDialog ui;
    ui.setupUi(&dialog);
    dialog.setWindowTitle("编辑题目" + mHead);
    ui.editQues->setPlainText(mText);
    for(const QString &str : mList) {
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
        // 清空原有内容
        mList.clear();
        int btnCnt = mLayoutButtons->count();
        for(int i = 0; i < btnCnt; i++) {
            mLayoutButtons->itemAt(i)->widget()->deleteLater();
        }

        // 重新创建选择按钮
        int count = ui.listWidget->count();
        for(int i = 0; i < count; ++i) {
            QString str = ui.listWidget->item(i)->text();
            mList << str;
            QAbstractButton *btn = createBtn(numToLetter(i) + ". " + str);
            connect(btn, &QAbstractButton::toggled, this, [this](bool) { emit changed(); });
            mLayoutButtons->addWidget(btn);
        }

        // 题目文字
        mText = ui.editQues->toPlainText();
        mLabelQues->setText(mHead + mText);
        return true;
    }
    return false;
}

void QuesChoice::writeXml(QXmlStreamWriter &xml) const {
    xml.writeStartElement(metaObject()->className());
    xml.writeAttribute("Ques", mText);

    int i = 0;
    // 遍历所有的选项文字，存入XML
    for(const QString &str : mList) {
        QAbstractButton *ansBtn = (QAbstractButton*)mLayoutButtons->itemAt(i)->widget();
        xml.writeStartElement("Ans");
        xml.writeAttribute("Checked", QString::number(ansBtn->isChecked()));
        xml.writeCharacters(str);
        xml.writeEndElement();
        i++;
    }

    xml.writeEndElement();
}
void QuesChoice::readXml(const QDomElement &elem) {
    mList.clear();
    int btnCnt = mLayoutButtons->count();
    for(int i = 0; i < btnCnt; i++) {
        mLayoutButtons->itemAt(i)->widget()->deleteLater();
    }

    mText = elem.attribute("Ques");
    mLabelQues->setText(mHead + mText);

    int i = 0;
    // 遍历所有子节点
    QDomNode node = elem.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        // 如果该节点名称为Ans，则读取文字并添加到选项中
        if(elem.tagName() == "Ans") {
            bool checked = elem.attribute("Checked").toInt();
            QString str = elem.text();
            mList << str;
            QAbstractButton *btn = createBtn(numToLetter(i) + ". " + str);
            btn->setChecked(checked);
            connect(btn, &QAbstractButton::toggled, this, [this](bool) { emit changed(); });
            mLayoutButtons->addWidget(btn);
            i++;
        }
        node = node.nextSibling();
    }
}

QString QuesChoice::numToLetter(int num) {
    QString res;
    do {
        res += 'A' + num % 26;
        num /= 26;
    } while(num);
    return res;
}

QAbstractButton* QuesChoice::createBtn(const QString &) { return nullptr; }
