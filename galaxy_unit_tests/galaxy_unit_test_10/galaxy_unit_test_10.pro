#-------------------------------------------------
#
# Project created by QtCreator 2014-07-31T18:21:35
#
#-------------------------------------------------

QT += core
CONFIG += debug_and_release
CONFIG += threads
QT       -= gui

TARGET = galaxy_unit_test_10
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app


SOURCES += main.cpp

HEADERS += \
    main.h

win32-g++: QMAKE_CXXFLAGS += -posix
