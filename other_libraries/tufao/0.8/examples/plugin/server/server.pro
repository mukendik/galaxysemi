TARGET = server
TEMPLATE = app

CONFIG += TUFAO0
QT -= gui

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += main.cpp \
    notfound.cpp

HEADERS += \
    notfound.h

RESOURCES += \
    static.qrc
