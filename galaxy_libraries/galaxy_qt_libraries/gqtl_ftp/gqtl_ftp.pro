MODULE=GQTL_FTP
include($(DEVDIR)/galaxy_common.pri)
# VERSION = 1.0.0.0
TEMPLATE = lib
CONFIG += debug_and_release
CONFIG += threads
CONFIG -= app_bundle
CONFIG += console

#TARGET = QtFtp
CONFIG += static
CONFIG -= shared
QT = core network
# -Wno-format-zero-length : To Do : how to add compiler options command line
TARGET = $$gexTargetName(gqtl_ftp)

#
LIBS += -Llib

#
# DESTDIR
#DESTDIR = lib/$$OSFOLDER
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
#load(qt_build_config)



MODULE_PRI = ../../modules/qt_ftp.pri
MODULE = ftp

#load(qt_module)

# Input
INCLUDEPATH += ../include/
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
HEADERS += ../include/qftp.h ../include/qurlinfo.h
SOURCES += qftp.cpp qurlinfo.cpp
