#include "mainview.h"
#include "ui_mainview.h"

MainView::MainView(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainView)
{
    ui->setupUi(this);

    connect(ui->btnNew, SIGNAL(clicked()), this, SIGNAL(newProjClicked()));
    connect(ui->btnLoad, SIGNAL(clicked()), this, SIGNAL(loadProjClicked()));
}
