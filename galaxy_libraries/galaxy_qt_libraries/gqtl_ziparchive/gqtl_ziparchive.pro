# -------------------------------------------------
# Project created by QtCreator 2009-03-25T11:19:11
# -------------------------------------------------
TEMPLATE = lib
CONFIG += staticlib debug_and_release

include($(DEVDIR)/galaxy_common.pri)
QT += widgets

DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl_ziparchive)

SOURCES += sources/gqtl_pkzip.cpp \
    sources/gqtl_pkstorage.cpp \
    sources/gqtl_pkinternalinfo.cpp \
    sources/gqtl_pkfileheader.cpp \
    sources/gqtl_pkcentraldir.cpp \
    sources/gqtl_pkautobuffer.cpp \
    sources/gqtl_archivefile.cpp \
    sources/gqtl_ansifile.cpp
#    source/gzlib.c

HEADERS += sources/gqtl_pkzip.h \
    sources/gqtl_pkstorage.h \
    sources/gqtl_pkinternalinfo.h \
    sources/gqtl_pkfileheader.h \
    sources/gqtl_pkcentraldir.h \
    sources/gqtl_pkautobuffer.h \
    ../include/gqtl_archivefile.h \
    ../include/gqtl_ansifile.h \
#    ../include/gqtl_zlib.h \
    ../include/gzguts.h \
    ../include/gqtl_zconf.h

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(QTSRCDIR)/qtbase/src/3rdparty/zlib \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include     # to be reviewed with gstdl_type.h; 20/09/2011, PYC, HT

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
	$$PWD/sources

CONFIG(debug, debug|release) {
	OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
	OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++*:DEFINES += _FILE_OFFSET_BITS=64
