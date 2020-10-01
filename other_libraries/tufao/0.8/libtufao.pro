QT       += network
QT       -= gui

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = lib/$$OSFOLDER

#TARGET = tufao # Apply GS rules for libs
TARGET = $$gexTargetName(tufao)

TEMPLATE = lib
# CONFIG += staticlib
CONFIG += exceptions # needed by HttpPluginServer::handleRequest(...)
CONFIG += console
DEFINES += BUFFER_SIZE=128
DEFINES += TUFAO_LIBRARY



#DEFINES += QT_VERSION=9999

! linux-g++-64:message($$DEFINES)

# TUFAO_EXPORT is defined in tufao_global.h
win32:CONFIG += shared
#win32:DEFINES+=QT_MAKEDLL

SOURCES += \
    src/websocket.cpp \
    src/urlrewriterhandler.cpp \
    src/url.cpp \
    src/simplesessionstore.cpp \
    src/sessionstore.cpp \
    src/querystring.cpp \
    src/httpupgraderouter.cpp \
    src/httpsserver.cpp \
    src/httpserverresponse.cpp \
    src/httpserverrequestrouter.cpp \
    src/httpserverrequest.cpp \
    src/httpserver.cpp \
    src/httppluginserver.cpp \
    src/httpfileserver.cpp \
    src/headers.cpp \
    src/abstractmessagesocket.cpp \
    src/abstracthttpupgradehandler.cpp \
    src/abstracthttpserverrequesthandler.cpp \
    src/priv/tcpserverwrapper.cpp \
    src/priv/rfc1123.cpp \
    src/priv/rfc1036.cpp \
    src/priv/reasonphrase.cpp \
    src/priv/asctime.cpp \
    src/priv/http_parser.c

HEADERS += \
    src/websocket.h \
    src/urlrewriterhandler.h \
    src/url.h \
    src/tufao_global.h \
    src/simplesessionstore.h \
    src/sessionstore.h \
    src/sessionsettings.h \
    src/session.h \
    src/querystring.h \
    src/ibytearray.h \
    src/httpupgraderouter.h \
    src/httpsserver.h \
    src/httpserverresponse.h \
    src/httpserverrequestrouter.h \
    src/httpserverrequest.h \
    src/httpserver.h \
    src/httppluginserver.h \
    src/httpfileserver.h \
    src/headers.h \
    src/abstractmessagesocket.h \
    src/abstracthttpupgradehandler.h \
    src/abstracthttpserverrequesthandlerfactory.h \
    src/abstracthttpserverrequesthandler.h \
    src/priv/websocket.h \
    src/priv/urlrewriterhandler.h \
    src/priv/url.h \
    src/priv/tcpserverwrapper.h \
    src/priv/simplesessionstore.h \
    src/priv/sessionstore.h \
    src/priv/rfc1123.h \
    src/priv/rfc1036.h \
    src/priv/reasonphrase.h \
    src/priv/httpupgraderouter.h \
    src/priv/httpsserver.h \
    src/priv/httpserverresponse.h \
    src/priv/httpserverrequestrouter.h \
    src/priv/httpserverrequest.h \
    src/priv/httpserver.h \
    src/priv/httppluginserver.h \
    src/priv/httpfileserver.h \
    src/priv/http_parser.h \
    src/priv/cryptography.h \
    src/priv/asctime.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
