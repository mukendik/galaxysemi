include($(DEVDIR)/galaxy_common.pri)

QT += network

CONFIG += console

TARGET = request-dumper
TEMPLATE = app

INCLUDEPATH += ../../include
INCLUDEPATH += ../../src

CONFIG += TUFAO0

LIBS += -L../../lib/$$OSFOLDER
LIBS += -l$$gexTargetName(tufao)

SOURCES += main.cpp mainhandler.cpp

HEADERS  += \
    mainhandler.h
