#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T08:13:19
#
#-------------------------------------------------

include($(DEVDIR)/galaxy_common.pri)

QT       += core
QT       -= gui

INCLUDEPATH += ../include

contains(QMAKE_COMPILER_DEFINES, _WIN64) {
    LIBS += -L../lib/win64
} else {
    LIBS += -L../lib/$$OSFOLDER
}

LIBS += -lR

TARGET = unit_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
