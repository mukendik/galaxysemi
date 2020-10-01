# -------------------------------------------------
# Project created by QtCreator 2009-03-25T11:05:25
# -------------------------------------------------
QT -= core \
    gui

TEMPLATE = lib
CONFIG += staticlib debug_and_release

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl_filelock)

SOURCES += sources/gqtl_filelock.cpp
HEADERS += ../include/gqtl_filelock.h

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
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
