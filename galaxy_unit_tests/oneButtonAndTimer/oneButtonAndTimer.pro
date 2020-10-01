#-------------------------------------------------
#
# Project created by QtCreator 2014-05-27T15:26:53
#
#-------------------------------------------------

QT       += core gui

CONFIG += debug_and_release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($(DEVDIR)/galaxy_warnings.pri)

TARGET = oneButtonAndTimer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
