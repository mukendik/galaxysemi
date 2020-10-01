#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T14:24:51
#
#-------------------------------------------------

QT       -= core

QT       -= gui

INCLUDEPATH  += ../libcfu-0.03/include


TARGET = ut2
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app


SOURCES += \
    large_hash.c \
    ../libcfu-0.03/src/cfu.c \
    ../libcfu-0.03/src/cfuhash.c \
    ../libcfu-0.03/src/cfulist.c \
    ../libcfu-0.03/src/cfuopt.c \
    ../libcfu-0.03/src/cfustring.c \
    ../libcfu-0.03/src/cfutime.c

HEADERS += \
    ../libcfu-0.03/include/cfuhash.h
