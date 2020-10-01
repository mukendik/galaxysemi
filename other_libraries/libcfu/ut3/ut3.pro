#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T21:20:22
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ut3
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

INCLUDEPATH  += ../libcfu-0.03/include

SOURCES += main.c \
    ../libcfu-0.03/src/cfu.c \
    ../libcfu-0.03/src/cfuhash.c \
    ../libcfu-0.03/src/cfulist.c \
    ../libcfu-0.03/src/cfuopt.c \
    ../libcfu-0.03/src/cfustring.c \
    ../libcfu-0.03/src/cfutime.c
