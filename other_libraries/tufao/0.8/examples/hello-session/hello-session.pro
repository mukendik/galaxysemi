TARGET = hello-session
TEMPLATE = app

CONFIG += TUFAO0

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += main.cpp \
    mainhandler.cpp

HEADERS  += \
    mainhandler.h
