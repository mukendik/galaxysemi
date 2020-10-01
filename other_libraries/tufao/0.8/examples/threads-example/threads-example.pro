TARGET = threads-example
TEMPLATE = app

CONFIG += TUFAO0
QT -= gui

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += main.cpp \
    mainhandler.cpp \
    tcpserver.cpp \
    handlerfactory.cpp \
    worker.cpp

HEADERS += mainhandler.h \
    tcpserver.h \
    handlerfactory.h \
    worker.h
