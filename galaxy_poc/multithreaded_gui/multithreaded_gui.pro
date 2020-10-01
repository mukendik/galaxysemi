
QT += core
QT += gui
QT += widgets
# QT += thread

TARGET = multithreaded_gui
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app

SOURCES += main.cpp \
    thread.cpp \
    widget.cpp

HEADERS += \
    thread.h \
    widget.h
