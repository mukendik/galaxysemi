#-------------------------------------------------
#
# Project created by QtCreator 2011-10-26T14:48:12
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = execution_flow_poc
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app


SOURCES += main.cpp

OTHER_FILES += \
    customers_message.xml
