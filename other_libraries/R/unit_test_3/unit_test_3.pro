#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T08:13:19
#
#-------------------------------------------------

QT       += core

QT       -= gui

INCLUDEPATH += ../include

LIBS += -L../lib/win32 -lR

TARGET = unit_test
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

SOURCES += main.cpp

