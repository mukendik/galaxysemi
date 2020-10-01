TARGET = webpost-echo-server
TEMPLATE = app

CONFIG += TUFAO0
QT -= gui

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += main.cpp \
    mainhandler.cpp \
    posthandler.cpp

HEADERS += mainhandler.h \
    posthandler.h

RESOURCES += \
    resources.qrc
