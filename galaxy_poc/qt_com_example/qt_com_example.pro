#-------------------------------------------------
#
# Project created by QtCreator 2015-12-11T17:05:07
#
#-------------------------------------------------

QT       += core gui
win32: QT += axcontainer

DEFINES += TERCOM
#DEFINES += USE_THREAD

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_com_example
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mycom.cpp

HEADERS  += mainwindow.h \
    mycom.h

FORMS    += mainwindow.ui
