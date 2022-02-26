#include "queschoice.h"
#include "ui_queschoiceeditdialog.h"

#include <QLabel>

#include <QVBoxLayout>
#include <QRandomGenerator>

#include <QXmlStreamWriter>
#include <QDomDocument>

QuesChoiceData::QuesChoiceData(const QString &quesName, QObject *parent)
    : QuesData(parent), quesName(quesName) {}
void QuesChoiceData::writeXml(QXmlStreamWriter &xml) const {
    xml.writeStartElement(quesName);
    xml.writeAttribute("Ques", quesText);

    int i = 0;
    // 遍历所有的选项文字，存入XML
    for(const Choice &choice : choiceList) {
        xml.writeStartElement("Ans");
        xml.writeAttribute("Checked", QString::number(choice.isChecked));
        xml.writeCharacters(choice.text);
        xml.writeEndElement();
        i++;
    }

    xml.writeEndElement();
}
void QuesChoiceData::readXml(const QDomElement &elem) {
    choiceList.clear();
    quesText = elem.attribute("Ques");

    // 遍历所有子节点
    QDomNode node = elem.firstChild();
    while(!node.isNull()) {
        QDomElement elem = node.toElement();
        // 如果该节点名称为Ans，则读取文字并添加到选项中
        if(elem.tagName() == "Ans") {
            bool checked = elem.attribute("Checked").toInt();
            QString str = elem.text();
            choiceList << Choice{ checked, str };
        }
        node = node.nextSibling();
    }
}
void QuesChoiceData::writeXmlWithoutTrueAns(QXmlStreamWriter &xml, int ind) const {
    xml.writeStartElement(quesName);
    xml.writeAttribute("Ind", QString::number(ind));
    xml.writeAttribute("Ques", quesText);

    // 遍历所有的选项文字，写入XML
    for(const Choice &choice : choiceList) {
        xml.writeTextElement("Ans", choice.text);
    }

    xml.writeEndElement();
}


QuesChoice::QuesChoice(const QString &quesName, const QString &head, QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel(head)),
      mLayout(new QVBoxLayout), mLayoutButtons(new QVBoxLayout),
      mHead(head), mData(quesName)
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
    ui.editQues->setPlainText(mData.quesText);
    for(const QuesChoiceData::Choice &choice : mData.choiceList) {
        QListWidgetItem *item = new QListWidgetItem(choice.text);
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
        mData.choiceList.clear();

        // 重新设定 choiceList
        int count = ui.listWidget->count();
        for(int i = 0; i < count; ++i) {
            QString str = ui.listWidget->item(i)->text();
            mData.choiceList << QuesChoiceData::Choice{ false, str };
        }

        // 题目文字
        mData.quesText = ui.editQues->toPlainText();

        updateWidgetsByData();
        return true;
    }
    return false;
}

void QuesChoice::writeXml(QXmlStreamWriter &xml) const {
    mData.writeXml(xml);
}
void QuesChoice::readXml(const QDomElement &elem) {
    mData.readXml(elem);
    updateWidgetsByData();
}

QString QuesChoice::numToLetter(int num) {
    QString res;
    do {
        res += 'A' + num % 26;
        num /= 26;
    } while(num);
    return res;
}

void QuesChoice::updateWidgetsByData() {
    // 题目文字
    mLabelQues->setText(mHead + mData.quesText);

    // 按钮
    int count = mLayoutButtons->count();
    for(int i = 0; i < count; ++i) {
        mLayoutButtons->itemAt(i)->widget()->deleteLater();
    }
    int i = 0;
    for(const QuesChoiceData::Choice &choice : mData.choiceList) {
        QAbstractButton *btn = createBtn(numToLetter(i) + ". " + choice.text);
        btn->setChecked(choice.isChecked);
        connect(btn, &QAbstractButton::toggled, this, [this, i](bool checked) {
            mData.choiceList[i].isChecked = checked;
            emit changed();
        });
        mLayoutButtons->addWidget(btn);
        ++i;
    }
}

QAbstractButton* QuesChoice::createBtn(const QString &) { return nullptr; }
