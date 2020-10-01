DEFINES += MODULE=GQTL_WS

QT = core
QT += network script
CONFIG	+= debug_and_release
TEMPLATE = lib

DEFINES += GQTL_WS_LIBRARY

win32:CONFIG += shared

include($(DEVDIR)/galaxy_common.pri)

TARGET	= $$gexTargetName(gqtl_ws)
DESTDIR	= $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

INCLUDEPATH += ../include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

# tufao
INCLUDEPATH += $(DEVDIR)/other_libraries/tufao/0.8/include
INCLUDEPATH += $(DEVDIR)/other_libraries/tufao/0.8/src

# path
LIBS += -L$(DEVDIR)/other_libraries/tufao/0.8/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
# libs
LIBS += -l$$gexTargetName(tufao)
# now needed for QObjectToJSON(...)
LIBS += -l$$gexTargetName(gqtl)

HEADERS += \
    ../include/gqtl_webserver.h \
    sources/gqtl_webserver_priv.h

SOURCES += \
    sources/gqtl_webserver.cpp \
    sources/gqtl_webserver_priv.cpp

CONFIG(debug, debug|release) {
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gstdl)
}
