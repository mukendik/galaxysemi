#-------------------------------------------------
#
# Project created by QtCreator 2014-09-02T16:01:20
#
#-------------------------------------------------

QT       += core script

QT       -= gui

include($(DEVDIR)/galaxy_warnings.pri)

TARGET = js_collect_garbage
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

HEADERS += \
    myclass.h
