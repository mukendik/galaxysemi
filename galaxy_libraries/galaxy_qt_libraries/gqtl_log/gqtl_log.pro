MODULE=GQTL_LOG

QT += network
QT += core
QT += xml
QT += sql
QT += script
# gqtl_log in NOT GUI and not responsible for GEX_ASSERT() (which could create a QMessageBox, so needs GUI)
QT -= gui

# VERSION = 1.0.0.0
TEMPLATE = lib
CONFIG += debug_and_release
CONFIG += threads
CONFIG -= app_bundle
CONFIG += console

include($(DEVDIR)/galaxy_common.pri)

# -Wno-format-zero-length : To Do : how to add compiler options command line
TARGET = $$gexTargetName(gqtl_log)

#
#LIBS += -Llib

#
# DESTDIR
#DESTDIR = lib/$$OSFOLDER
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

#
#DLLDESTDIR = $$PWD/../bin
#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin

# no more needed, should be done by galaxy_common
#win32:LIBS += libwsock32 \
#    libws2_32
#
DEFINES += GQTLLOG_LIBRARY
INCLUDEPATH += ../include/
INCLUDEPATH += include/
INCLUDEPATH += sources/
INCLUDEPATH += .
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
SOURCES += sources/gqtl_log.cpp \
    sources/LogFile.cpp \
    sources/to_rtf.cpp \
    sources/to_xml.cpp \
    sources/to_syslog.cpp \
    sources/to_sql.cpp \
    sources/to_csv.cpp \
    sources/to_txt.cpp \
    sources/to_console.cpp \
    sources/coutput.cpp
#
HEADERS += ../include/gqtl_log.h \
    sources/coutput.h

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

# $(DESTDIR) is lib/win32/
win32 {
#QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/$$TARGET* $$PWD/../gex-httpserver/
}

CONFIG(debug, debug|release) {
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gstdl)
}
