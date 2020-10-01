#-------------------------------------------------
#
# Project created by QtCreator 2010-10-11T16:18:00
#
#-------------------------------------------------
#LibNameForLink=gexpb
# -------------------------------------------------
#
DEFINES += MODULE=GEXPB
#
QT       += xml
# needs gex script engine
QT       += script
QT += widgets
#
TEMPLATE = lib
#
DEFINES += GEXPB_LIBRARY
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

# To remove warning : auto-importing has been activated without --enable-auto-import specified on the command line.
#QMAKE_LDFLAGS += -Wl,--enable-auto-import
#QMAKE_LFFLAGS += -Wl,--enable-auto-import
win32 {
QMAKE_LFLAGS += -Wl,--enable-auto-import
message( Fixing QMAKE_LFLAGS = $$QMAKE_LFLAGS )
}

#
TARGET = $$gexTargetName(gexpb)
# DESTDIR
#DESTDIR = lib/$$OSFOLDER
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/gex-pb/lib/$$OSFOLDER
#
#DLLDESTDIR = $$PWD/../bin
#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin

# qtpropertybrowser
QTPBLIB = -L$(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/$$OSFOLDER

LIBS += $$GALAXY_LIBS
LIBS += $$GALAXY_LIBSPATH

LIBS += $$QTPBLIB \
    -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER


include($(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src/qtpropertybrowser.pri)
#

INCLUDEPATH += ./
INCLUDEPATH += ./sources
INCLUDEPATH += ./include
INCLUDEPATH += $(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
#
DEPENDPATH += $(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src
#
SOURCES += sources/cgexpb.cpp \
    sources/varman.cpp \
    sources/vareditor.cpp \
    sources/propbrowser.cpp \
    sources/cgexpublicproperty.cpp \
    sources/cpropertyidmanager.cpp \
    sources/pb_directory_widget.cpp
#
HEADERS +=\
	include/libgexpb.h \
	sources/propbrowser.h \
    sources/cgexpublicproperty.h \
    sources/cpropertyidmanager.h \
    sources/cgexpb.h \
    sources/pb_directory_widget.h

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

#unix {
#	QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $$DLLDESTDIR
#}
