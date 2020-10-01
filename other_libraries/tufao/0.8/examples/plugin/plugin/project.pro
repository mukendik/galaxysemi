TARGET = plugin
TEMPLATE = lib

QT -= gui
QT += core
CONFIG += plugin TUFAO0

include($(DEVDIR)/galaxy_warnings.pri)

DEFINES += PLUGIN=%1

SOURCES += plugin.cpp \
    requesthandler.cpp

HEADERS += plugin.h \
    requesthandler.h
