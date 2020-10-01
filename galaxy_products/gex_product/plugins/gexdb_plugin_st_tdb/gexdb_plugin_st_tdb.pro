# -------------------------------------------------
# Project created by QtCreator 2009-05-05T15:04:27
# -------------------------------------------------
QT += network \
    sql \
    qt3support
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/plugins/lib
TARGET = gexdb_plugin_st_tdb
TEMPLATE = lib
DEFINES += GEXDB_PLUGIN_EXPORTS
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += sources/gexdb_plugin_st_tdb_cfgwizard.cpp \
    sources/gexdb_plugin_st_tdb.cpp
HEADERS += sources/gexdb_plugin_st_tdb_cfgwizard.h \
    sources/gexdb_plugin_st_tdb.h \
    sources/gexdb_plugin_st_tdb_constants.h
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
	$(DEVDIR)/galaxy_products/gex_product/gex/sources \
	$(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources/ui \
	$(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_st_tdb/sources/ui
FORMS += sources/gexdb_plugin_st_tdb_cfg_wizard.ui
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
	$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
	$(DEVDIR)/galaxy_products/gex_product/include \
	$(DEVDIR)/galaxy_products/gex_product/plugins/include \
	$(DEVDIR)/galaxy_products/gex_product/gex/sources \
	$(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources \
	$$PWD/sources
UI_DIR = $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_st_tdb/sources/ui

GALAXY_LIBS = -lgexdb_plugin_base -lgstdl_utils_c \
    -lgstdl_utils \
    -lgstdl_errormgr \
    -lgstdl_blowfish_c \
    -lgqtl_sysutils

GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib \
    -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib \
    -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib

CONFIG(debug, debug|release) { 
    DESTDIR = $$join(DESTDIR,"_debug ",,_debug)
    TARGET = $$join(TARGET,"_d ",,_d)
    GALAXY_LIBS = $$join(GALAXY_LIBS,"_d ",,_d)
    GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"_debug ",,_debug)

	DLLDESTDIR = $$PWD/../../bin_debug/plugins
	#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin_debug/plugins
} else {
	#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin/plugins
        DLLDESTDIR = $$PWD/../../bin/plugins/db
}

# Set the directory according to the Operating System
CONFIG(production) {
	win32:DLLDESTDIR = $$join(DLLDESTDIR,,,/win32)
	linux-g++:DLLDESTDIR = $$join(DLLDESTDIR,,,/linux)
	solaris-g++:DLLDESTDIR = $$join(DLLDESTDIR,,,/solaris)

	win32:DESTDIR = $$join(DESTDIR,,,/win32)
	linux-g++:DESTDIR = $$join(DESTDIR,,,/linux)
	solaris-g++:DESTDIR = $$join(DESTDIR,,,/solaris)

	win32:OBJECTS_DIR = $$PWD/release/win32/
	linux-g++:OBJECTS_DIR = $$PWD/release/linux/
	solaris-g++:OBJECTS_DIR = $$PWD/release/solaris/

	win32:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/win32 ",,/win32)
	linux-g++:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/linux ",,/linux)
	solaris-g++:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/solaris ",,/solaris)
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH

win32:LIBS += -lws2_32
unix:DESTDIR=$$DLLDESTDIR

exists(user_custom.pri) {
	include(user_custom.pri)
}

linux-g++:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
