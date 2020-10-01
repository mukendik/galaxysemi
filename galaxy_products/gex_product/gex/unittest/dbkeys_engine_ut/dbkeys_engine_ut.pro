#-------------------------------------------------
#
# Project created by QtCreator 2012-07-04T15:17:04
#
#-------------------------------------------------

QT       += core gui xml widgets

TARGET = dbkeys_engine_ut
CONFIG   += console
#CONFIG   += debug_and_release
#CONFIG   -= app_bundle

TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $$PWD/bin
TARGET = $$gexTargetName(dbkeys_engine_ut)

SOURCES += main.cpp \
            dbkeyunittest.cpp \
            ../../../plugins/gexdb_plugin_base/sources/db_datakeys.cpp \
            ../../../plugins/gexdb_plugin_base/sources/database_keys_engine.cpp

HEADERS += dbkeysunittest.h \
    ../../../plugins/include/db_datakeys.h \
    ../../../plugins/include/database_keys_engine.h

INCLUDEPATH =   $(DEVDIR)/galaxy_products/gex_product/include \
                $(DEVDIR)/galaxy_products/gex_product/plugins/include \
                $(DEVDIR)/galaxy_products/gex_product/gex/sources \
                $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
                $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
                $(QTSRCDIR)

DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

#
GALAXY_LIBS = -l$$gexTargetName(gqtl)

GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

# plugin_base must be before -lws2_32
GALAXY_LIBS += -l$$gexTargetName(gexdb_plugin_base)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER

LIBS += $$GALAXY_LIBS \
        $$GALAXY_LIBSPATH
