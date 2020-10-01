#-------------------------------------------------
#
# Project created by QtCreator 2014-01-15T10:28:04
#
#-------------------------------------------------

QT += core
QT += network \
    sql \
    xml \
    webkit \
    script
QT += gui
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DEFINES += QT_HASH_SEED=1
QT += printsupport
QT += webkitwidgets
QT += printsupport
QT += opengl
QT += scripttools

QMAKE_CXXFLAGS += -mfpmath=sse -march=pentium4

TARGET = new_poc

CONFIG += console
# CONFIG -= app_bundle ?
CONFIG += threads
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    test.h
