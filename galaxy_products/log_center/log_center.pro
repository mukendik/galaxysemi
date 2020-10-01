# -------------------------------------------------
QT += network
QT += core
QT += xml
QT += sql
QT += gui
QT += widgets

TEMPLATE = app
CONFIG += debug_and_release
CONFIG += threads
CONFIG -= app_bundle
#CONFIG += console

include($(DEVDIR)/galaxy_common.pri)

# -Wno-format-zero-length : To Do : how to add compiler options command line
TARGET = $$gexTargetName(gs-logcenter)

#
#LIBS += -Llib
#
# DESTDIR
#DESTDIR = lib/$$OSFOLDER
DESTDIR = $(DEVDIR)/galaxy_products/log_center/$$OSFOLDER
#
win32:LIBS += libwsock32 \
    libws2_32
#
INCLUDEPATH += ../include/
INCLUDEPATH += include/
INCLUDEPATH += src/
INCLUDEPATH += .
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

SOURCES += src/logwidget.cpp \
    src/main.cpp \
    src/syslog_server.cpp

HEADERS += include/logwidget.h \
    include/syslog_server.h

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

#OTHER_FILES += \
 #   libgexlog.rc

#unix {
#	QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $$DLLDESTDIR
#}

# $(DESTDIR) is lib/win32/
win32 {
#QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/$$TARGET* $$PWD/../gex-httpserver/
}

RESOURCES += \
    logs_center.qrc

LIBS += -l$$gexTargetName(gstdl)
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
