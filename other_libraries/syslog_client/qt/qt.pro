TEMPLATE = app
CONFIG += console
CONFIG -= qt

include($(DEVDIR)/galaxy_warnings.pri)

INCLUDEPATH += ..

LIBS += -lwsock32

SOURCES += \
    ../demo.c \
    ../syslog.c \
    ../printk.c

HEADERS += \
    ../syslog.h

