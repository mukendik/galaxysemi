#-------------------------------------------------
#
# Project created by QtCreator 2012-03-29T10:22:37
#
#-------------------------------------------------

QT       += core gui

include($(DEVDIR)/galaxy_warnings.pri)

TARGET = tufao-routes-editor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    plugindialog.cpp \
    datahandler.cpp

HEADERS  += mainwindow.h \
    plugindialog.h \
    datahandler.h

FORMS    += mainwindow.ui \
    plugindialog.ui
