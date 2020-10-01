#LibNameForLink=gex-httpserver(app)
#CalledLib=gqtl_sysutils
# -------------------------------------------------
QT += network \
    webkit
#QT -= gui
TARGET = gex-httpserver
DEFINES += MODULE=gexhttpserver
#
INCLUDEPATH += qtservice
INCLUDEPATH += ../../../galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += ../gex-log/include

win32:LIBS += -L../../../galaxy_libraries/galaxy_qt_libraries/lib/win32/

LIBS += libgqtl_sysutilsd
#
CONFIG += console
CONFIG -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app
SOURCES += main.cpp \
    qtservice/qtservice_win.cpp \
    qtservice/qtservice.cpp \
    httpdaemon.cpp \
    ../gex-log/include/libgexlog_public.cpp \
    httpservice.cpp
OTHER_FILES += qtservice/QtServiceController \
    qtservice/QtServiceBase
HEADERS += qtservice/qtunixserversocket.h \
    qtservice/qtservice_p.h \
    qtservice/qtservice.h \
    qtservice/qtunixsocket.h \
    httpservice.h \
    httpdaemon.h
