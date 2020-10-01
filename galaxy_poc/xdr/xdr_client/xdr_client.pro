#-------------------------------------------------
#
# Project created by QtCreator 2014-12-03T16:11:36
#
#-------------------------------------------------

QT       += core

QT       -= gui

INCLUDEPATH += $(DEVDIR)/other_libraries/xdr/bsd-xdr-1.0.0

TARGET = xdr_client
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

win32 {
LIBS += -lws2_32 -lsnmpapi
}

SOURCES += \
    ../client.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_array.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_float.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_mem.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_private.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_rec.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_reference.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_sizeof.c \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_stdio.c

HEADERS += \
    ../../../other_libraries/xdr/bsd-xdr-1.0.0/lib/xdr_private.h
