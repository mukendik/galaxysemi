TARGET = websocket-chat
TEMPLATE = app

CONFIG += TUFAO0
QT -= gui

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += main.cpp \
    webserver.cpp

OTHER_FILES += \
    index.html

HEADERS += \
    webserver.h

RESOURCES += \
    resources.qrc
