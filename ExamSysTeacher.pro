#-------------------------------------------------
#
# Project created by QtCreator 2022-02-06T20:34:50
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ExamSysTeacher
TEMPLATE = app

# QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        Ques/ques.cpp \
        Ques/queschoice.cpp \
        Ques/quesmultichoice.cpp \
        Ques/quessinglechoice.cpp \
        Ques/queswhether.cpp \
        SubWidget/editview.cpp \
        SubWidget/mainview.cpp \
        SubWidget/pushview.cpp \
        Util/config.cpp \
        Widget/doubleslidebutton.cpp \
        Widget/examhistory.cpp \
        Widget/examhistoryitemwidget.cpp \
        Widget/examwidget.cpp \
        Widget/scorewidget.cpp \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        Ques/ques.h \
        Ques/queschoice.h \
        Ques/quesmultichoice.h \
        Ques/quessinglechoice.h \
        Ques/queswhether.h \
        SubWidget/editview.h \
        SubWidget/mainview.h \
        SubWidget/pushview.h \
        Util/config.h \
        Util/header.h \
        Widget/doubleslidebutton.h \
        Widget/examhistory.h \
        Widget/examhistoryitemwidget.h \
        Widget/examwidget.h \
        Widget/scorewidget.h \
        mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    Ques/queschoiceeditdialog.ui \
    Ques/queswhetherdialog.ui \
    SubWidget/addquesdialog.ui \
    SubWidget/editview.ui \
    SubWidget/mainview.ui \
    SubWidget/pushview.ui \
    Widget/examhistory.ui \
    Widget/examwidget.ui \
    mainwindow.ui

RESOURCES += \
    src.qrc
