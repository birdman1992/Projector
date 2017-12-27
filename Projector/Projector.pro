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
    protask.cpp \
    Qextserial/qextserialport_unix.cpp \
    Qextserial/qextserialport.cpp \
    gprsnetwork.cpp \
    videoplayer.cpp \
    sysdev.cpp \

HEADERS  += mainwidget.h \
    network.h \
    Json/cJSON.h \
    QTextScroll/qtextscroll.h \
    protask.h \
    Qextserial/qextserialport_global.h \
    Qextserial/qextserialport_p.h \
    Qextserial/qextserialport.h \
    gprsnetwork.h \
    videoplayer.h \
    sysdev.h \

FORMS    += mainwidget.ui
