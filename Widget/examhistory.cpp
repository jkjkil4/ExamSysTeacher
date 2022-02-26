#include "examhistory.h"
#include "ui_examhistory.h"

#include <QDir>
#include <QXmlStreamReader>
#include <QDateTime>

#include "Util/header.h"
#include "examhistoryitemwidget.h"

ExamHistory::ExamHistory(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ExamHistory)
{
    ui->setupUi(this);

    struct Exam {
        QString dirName, name;
        QDateTime dtCreate, dtEnd;
        bool isEnd;
    };
    QList<Exam> mExamList;

    QStringList dirs = QDir(APP_DIR + "/Exported").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(const QString &dirName : qAsConst(dirs)) {
        QFile file(APP_DIR + "/Exported/" + dirName + "/_.esep");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QXmlStreamReader xml(&file);
        if(xml.readNextStartElement()) {
            QString name = xml.attributes().value("Name").toString();
            QDateTime dtCreate = QDateTime::fromString(xml.attributes().value("CreateDateTime").toString(), "yyyy/M/d H:m:s");
            QDateTime dtEnd = QDateTime::fromString(xml.attributes().value("EndDateTime").toString(), "yyyy/M/d H:m:s");
            mExamList << Exam{ dirName, name, dtCreate, dtEnd, QDateTime::currentDateTime() >= dtEnd };
        }
        file.close();
    }

    std::sort(mExamList.begin(), mExamList.end(), [](const Exam &a, const Exam &b) -> bool {
        if(a.isEnd != b.isEnd)
            return a.isEnd;
        return a.dtCreate > b.dtCreate;
    });

    for(const Exam &exam : mExamList) {
        ExamHistoryItemWidget *widget = new ExamHistoryItemWidget(exam.dirName, exam.name, exam.dtCreate, exam.isEnd);
        widget->adjustSize();
        QListWidgetItem *item = new QListWidgetItem;
        item->setSizeHint(widget->size());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);
    }

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item) {
        mExamDirName = ((ExamHistoryItemWidget*)ui->listWidget->itemWidget(item))->dirName();
        accept();
    });
    connect(ui->btnEnter, &QPushButton::clicked, [this] {
        QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
        if(items.isEmpty())
            return;
        mExamDirName = ((ExamHistoryItemWidget*)ui->listWidget->itemWidget(items[0]))->dirName();
        accept();
    });
    connect(ui->btnCancel, &QPushButton::clicked, this, &ExamHistory::reject);
}
