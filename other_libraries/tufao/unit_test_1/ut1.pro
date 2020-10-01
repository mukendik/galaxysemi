include($(DEVDIR)/galaxy_common.pri)

QT += network script

CONFIG += console

TARGET = ut1
TEMPLATE = app

INCLUDEPATH += ../0.8/include
INCLUDEPATH += ../0.8/src

CONFIG += TUFAO0

LIBS += -L../0.8/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(tufao)

SOURCES += main.cpp mainhandler.cpp

HEADERS  += mainhandler.h
