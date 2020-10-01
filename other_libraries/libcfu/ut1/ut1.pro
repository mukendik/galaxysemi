#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T13:34:32
#
#-------------------------------------------------

QT       -= core

QT       -= gui

TARGET = ut1
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

INCLUDEPATH  += ../libcfu-0.03/include

SOURCES += \
    ../libcfu-0.03/src/cfu.c \
    ../libcfu-0.03/src/cfuconf.c \
    ../libcfu-0.03/src/cfuhash.c \
    ../libcfu-0.03/src/cfutime.c \
    ../libcfu-0.03/src/cfulist.c \
    ../libcfu-0.03/src/cfustring.c \
    hash_usage.c
