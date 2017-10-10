#-------------------------------------------------
#
# Project created by QtCreator 2017-10-09T13:52:00
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Projector
TEMPLATE = app


SOURCES += main.cpp\
        mainwidget.cpp \
    network.cpp \
    Json/cJSON.c \
    QTextScroll/qtextscroll.cpp \
    protask.cpp

HEADERS  += mainwidget.h \
    network.h \
    Json/cJSON.h \
    QTextScroll/qtextscroll.h \
    protask.h

FORMS    += mainwidget.ui
