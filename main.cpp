#include "mainwindow.h"
#include <QApplication>

#include "Widget/doubleslidebutton.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();

    DoubleSlideButton btn;
    btn.show();

    return a.exec();
}
