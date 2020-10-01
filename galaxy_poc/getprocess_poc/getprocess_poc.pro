#-------------------------------------------------
#
# Project created by QtCreator 2014-11-04T16:51:43
#
#-------------------------------------------------

QT       += core

QT       -= gui

include($(DEVDIR)/galaxy_common.pri)

message($$GALAXY_LIBS)

LIBS += $$GALAXY_LIBSPATH
LIBS += $$GALAXY_LIBS

TARGET = getprocess_poc
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp
