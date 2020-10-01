# -------------------------------------------------
# Project created by QtCreator 2009-06-05T09:40:52
# -------------------------------------------------
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/plugins/lib
TARGET = gexpat_plugin_demo
TEMPLATE = lib
DEFINES += GEXPAT_PLUGIN_EXPORTS
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES += sources/gexpat_plugin_demo.cpp
HEADERS += sources/gexpat_plugin_demo.h
INCLUDEPATH = $(DEVDIR)/galaxy_products/gex_product/plugins/include
DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/include \
	$$PWD/sources
GALAXY_LIBS = -lgexpat_plugin_base
GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib
CONFIG(debug, debug|release) { 
    DESTDIR = $$join(DESTDIR,"_debug ",,_debug)
	TARGET = $$join(TARGET,"_d ",,_d)
    GALAXY_LIBS = $$join(GALAXY_LIBS,"_d ",,_d)
    GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"_debug ",,_debug)

        DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin_debug/plugins/pat

} else {
        DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin/plugins/pat
}

# Set the directory according to the Operating System
CONFIG(production) {
	win32:DLLDESTDIR = $$join(DLLDESTDIR,,,/win32)
	linux-g++:DLLDESTDIR = $$join(DLLDESTDIR,,,/linux)
	solaris-g++:DLLDESTDIR = $$join(DLLDESTDIR,,,/solaris)
}

win32:DESTDIR = $$join(DESTDIR,,,/win32)
linux-g++:DESTDIR = $$join(DESTDIR,,,/linux)
solaris-g++:DESTDIR = $$join(DESTDIR,,,/solaris)

win32:OBJECTS_DIR = $$PWD/release/win32/
linux-g++:OBJECTS_DIR = $$PWD/release/linux/
solaris-g++:OBJECTS_DIR = $$PWD/release/solaris/

win32:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/win32 ",,/win32)
linux-g++:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/linux ",,/linux)
solaris-g++:GALAXY_LIBSPATH = $$join(GALAXY_LIBSPATH,"/solaris ",,/solaris)

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH
unix:DESTDIR=$$DLLDESTDIR

exists(user_custom.pri) {
	include(user_custom.pri)
}

linux-g++:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
