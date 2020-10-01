TEMPLATE = app
CONFIG += console
CONFIG -= qt

include($(DEVDIR)/galaxy_warnings.pri)

win32:LIBS += libwsock32 libws2_32 libComdlg32

DEFINES += USE_WEBSOCKET

INCLUDEPATH += .

SOURCES += \
    mongoose.c \
#    main.c \ # a simple example with dir listing
    examples/websocket.c

HEADERS += \
    mongoose.h

