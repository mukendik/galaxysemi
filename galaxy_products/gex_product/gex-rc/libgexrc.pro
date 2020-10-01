# -------------------------------------------------
#CalledLib=gexrc
#CalledLib=gqtl_filelock
#CalledLib=gqtl_service
#CalledLibexternal=ws2_32
#CalledLibexternal=advapi32
# -------------------------------------------------
QT += core
QT += xml
QT += gui

include($(DEVDIR)/galaxy_common.pri)

TEMPLATE = lib
CONFIG += debug_and_release
CONFIG += threads
CONFIG -= app_bundle
CONFIG += console

#TARGET = libgexrc
TARGET = $$gexTargetName(gexrc)

LIBS += -Llib
#DESTDIR = lib
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/gex-rc/lib/$$OSFOLDER


CONFIG(debug, debug|release):TARGET = $$join(TARGET,,,_d)

# DESTDIR = $$join(DESTDIR,,,_debug)
#win32:DESTDIR = $$join(DESTDIR,,,/win32)
#linux-g++:DESTDIR = $$join(DESTDIR,,,/linux)
#solaris-g++:DESTDIR = $$join(DESTDIR,,,/solaris)

DEFINES += LIBGEXRC_LIBRARY
INCLUDEPATH += include/
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
#INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-log/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += src/
INCLUDEPATH += .

SOURCES += sources/libgexrc.cpp \
    ../gex-log/include/libgexlog_public.cpp
HEADERS += include/libgexrc.h

OTHER_FILES +=
