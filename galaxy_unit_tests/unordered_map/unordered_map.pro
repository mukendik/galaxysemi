#-------------------------------------------------
#
# Project created by QtCreator 2014-07-05T00:53:12
#
#-------------------------------------------------

QT       -= core
QT       -= gui

TARGET = unordered_map
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
SOURCES += main.cpp
