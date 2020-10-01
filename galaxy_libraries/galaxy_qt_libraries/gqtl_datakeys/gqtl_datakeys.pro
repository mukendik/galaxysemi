#-------------------------------------------------
#
# Project created by QtCreator 2013-12-11T14:33:37
#
#-------------------------------------------------
DEFINES += MODULE=GQTL_DATAKEYS
QT += xml
QT       -= gui

TEMPLATE = lib
CONFIG += debug_and_release

DEFINES += GQTL_DATAKEYS_LIBRARY

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl_datakeys)

SOURCES += \
    sources/gqtl_datakeys_file.cpp \
    sources/gqtl_datakeys_engine.cpp \
    sources/gqtl_datakeys_content.cpp \
    sources/gqtl_datakeys_loader.cpp \
    sources/gqtl_datakeys_definition_loader.cpp \
    sources/gqtl_datakeys_data.cpp

HEADERS += \
    ../include/gqtl_datakeys_global.h \
    ../include/gqtl_datakeys_file.h \
    ../include/gqtl_datakeys_engine.h \
    ../include/gqtl_datakeys_content.h \
    ../include/gqtl_datakeys.h \
    ../include/gqtl_datakeys_loader.h \
    ../include/gqtl_datakeys_definition_loader.h \
    ../include/gqtl_datakeys_data.h

LIBS +=\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/bin\
 -l$$gexTargetName(gqtl)\
 -l$$gexTargetName(gqtl_stdf)\
 -l$$gexTargetName(gstdl)\
 -lgs_data

INCLUDEPATH = \
$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
$(DEVDIR)/other_libraries/sqlite/3.7.17

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
            $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
            $$PWD/sources

CONFIG(debug, debug|release) {
        OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
        OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

RESOURCES += \
    sources/datakeys.qrc
