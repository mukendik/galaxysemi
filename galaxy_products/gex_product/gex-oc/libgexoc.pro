# -------------------------------------------------
QT += core
QT += xml
QT += gui
QT += script
QT += widgets
#
DEFINES += MODULE="GEXOC"

TEMPLATE = lib
#
CONFIG += debug_and_release
CONFIG += threads
CONFIG -= app_bundle
CONFIG += console

include($(DEVDIR)/galaxy_common.pri)

#
TARGET = $$gexTargetName(gexoc)
LIBS += -Llib
#DESTDIR = lib/$$OSFOLDER
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/gex-oc/lib/$$OSFOLDER

# DLLDESTDIR cant use $(DEVDIR) on Windows7 because it badly removes all '\'...
#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
#DLLDESTDIR = ../bin

#include($(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src/qtpropertybrowser.pri)

DEFINES += LIBGEXOC_LIBRARY

INCLUDEPATH += include/
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
#INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-log/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-pb/include
#INCLUDEPATH += $(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += src/
INCLUDEPATH += .

SOURCES += sources/libgexoc.cpp \
    sources/options_center_widget.cpp \
#    ../gex-log/include/libgexlog_public.cpp \
    sources/options_center_propbrowser.cpp

HEADERS += include/libgexoc.h \
    include/options_center_widget.h \
    include/options_center_propbrowser.h

GEXPB_LIBS = -l$$gexTargetName(gexpb)

GEXPB_LIBSPATH = -L$(DEVDIR)/galaxy_products/gex_product/gex-pb/lib/$$OSFOLDER

# add _d to all libs
#CONFIG(debug, debug|release) {
#	GEXPB_LIBS = $$join(GEXPB_LIBS,"_d ",,_d)
    #GEXPB_LIBSPATH = $$join(GEXPB_LIBSPATH,"_debug ",,_debug)
#}
#
LIBS += $$GEXPB_LIBS \
    $$GEXPB_LIBSPATH \
    -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \
#    -l$$gexTargetName(gqtl_sysutils)
    -l$$gexTargetName(gqtl)

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

linux*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

#unix {
#	QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $$DLLDESTDIR
#}

win32:LIBS += -lPsapi

CONFIG(debug, debug|release) {
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gstdl)
}
