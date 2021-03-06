#include "queschoice.h"
#include "ui_queschoiceeditdialog.h"

#include <QLabel>

#include <QVBoxLayout>
#include <QRandomGenerator>

#include <QXmlStreamWriter>
#include <QDomDocument>

inline QString numToLetter(int num) {
    QString res;
    do {
        res += (char)('A' + num % 26);
        num /= 26;
    } while(num);
    return res;
}

QuesChoiceData::QuesChoiceData(const QString &quesName, const QString &head, QObject *parent)
    : QuesData(parent), quesName(quesName), head(head) {}
QString QuesChoiceData::ansStr() const {
    QString str;
    // 遍历 choiceList 将选中的索引写入到 str 中
    int i = 0;
    for(const Choice &choice : choiceList) {
        if(choice.isChecked)
            str += QString::number(i) + ';';
        ++i;
    }
    return str;
}
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
void QuesChoiceData::writeXmlWithoutTrueAns(QXmlStreamWriter &xml) const {
    xml.writeStartElement(quesName);
    xml.writeAttribute("Ques", quesText);

    // 遍历所有的选项文字，写入XML
    for(const Choice &choice : choiceList) {
        xml.writeTextElement("Ans", choice.text);
    }

    xml.writeEndElement();
}
void QuesChoiceData::writeXmlTrueAns(QXmlStreamWriter &xml) const {
    xml.writeTextElement("v", ansStr());
}
QuesChoiceData::Score QuesChoiceData::score(const QString &str) const {
    bool isRight = str == ansStr();
    QString html;

    if(!isRight) {
        html += "<font color=\"red\">" + head + "</font>" + quesText;
    } else html += head + quesText;

    bool *arrIsStuChecked = new bool[choiceList.size()] { 0 };
    QStringList list = str.split(';', QString::SkipEmptyParts);
    for(const QString &part : qAsConst(list)) {
        int num = part.toInt();
        if(num >= 0 && num < choiceList.size())
            arrIsStuChecked[num] = true;
    }

    int i = 0;
    for(const Choice &choice : choiceList) {
        html += "<br>&nbsp;&nbsp;&nbsp;";
        html += "<span style=\"";
        if(choice.isChecked)
            html += "color: #22aa22;";
        if(arrIsStuChecked[i])
            html += "text-decoration:underline;";
        html += "\">";
        html += "&nbsp;" + numToLetter(i) + ". " + choice.text.toHtmlEscaped() + "&nbsp;";
        html += "</span>";
        ++i;
    }

    delete[] arrIsStuChecked;

    return Score{ isRight, html };
}


QuesChoice::QuesChoice(const QString &quesName, const QString &head, QWidget *parent)
    : Ques(parent),
      mLabelQues(new QLabel(head)),
      mLayout(new QVBoxLayout), mLayoutButtons(new QVBoxLayout),
      mHead(head), mData(quesName, head)
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
