# -------------------------------------------------
# Project created by QtCreator 2009-03-25T11:05:25
# -------------------------------------------------
DEFINES += MODULE=GQTL

QT += core \
    gui \
    widgets

TEMPLATE = lib
CONFIG += staticlib debug_and_release

include($(DEVDIR)/galaxy_common.pri)

#DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
DESTDIR = $$PWD/lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl)

SOURCES += \
    gqtl_sysutils/sources/gqtl_sysutils.cpp \
    gqtl_sysutils/sources/gqtl_skinstyle.cpp \
    gqtl_sysutils/sources/gqtl_skin.cpp \
    gqtl_utils/sources/gqtl_utils.cpp \
    gqtl_ziparchive/sources/gqtl_pkzip.cpp \
    gqtl_ziparchive/sources/gqtl_pkstorage.cpp \
    gqtl_ziparchive/sources/gqtl_pkinternalinfo.cpp \
    gqtl_ziparchive/sources/gqtl_pkfileheader.cpp \
    gqtl_ziparchive/sources/gqtl_pkcentraldir.cpp \
    gqtl_ziparchive/sources/gqtl_pkautobuffer.cpp \
    gqtl_ziparchive/sources/gqtl_archivefile.cpp \
    gqtl_ziparchive/sources/gqtl_ansifile.cpp \
    gqtl_utils/sources/range.cpp \
    gqtl_core/sources/multi_limit_item.cpp \
    gqtl_core/sources/gqtl_global.cpp \
    gqtl_utils/sources/number_format.cpp \
    gqtl_core/sources/qx_string_list.cpp

HEADERS += \
    gqtl_sysutils/sources/gqtl_skinstyle.h \
    gqtl_ziparchive/sources/gqtl_pkzip.h \
    gqtl_ziparchive/sources/gqtl_pkstorage.h \
    gqtl_ziparchive/sources/gqtl_pkinternalinfo.h \
    gqtl_ziparchive/sources/gqtl_pkfileheader.h \
    gqtl_ziparchive/sources/gqtl_pkcentraldir.h \
    gqtl_ziparchive/sources/gqtl_pkautobuffer.h \
    include/multi_limit_item.h \
    include/test_defines.h \
    include/gqtl_archivefile.h \
    include/gqtl_ansifile.h \
#    include/gqtl_zlib.h \
#    include/gqtl_zconf.h \
    include/gqtl_sysutils.h \
    include/gqtl_utils.h \
    include/gqtl_global.h \
    include/qx_string_list.h

INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/zlib

INCLUDEPATH += \
$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
                $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
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
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

# To be able to use GetProcessMemoryInfo() on Windows 7 and earlier (Win XP)
win32:DEFINES += PSAPI_VERSION=1

LIBS += -lpsapi -lkernel32
