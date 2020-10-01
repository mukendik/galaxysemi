#-------------------------------------------------
#
# Project created by QtCreator 2013-01-03T10:51:38
#
#-------------------------------------------------

QT += core
QT -= gui

include($(DEVDIR)/galaxy_common.pri)

TEMPLATE = lib

CONFIG += debug_and_release
CONFIG -= staticlib
CONFIG -= app_bundle
CONFIG += console
CONFIG += dll

TARGET = gtl-igxl

DEFINES += LIBGTLIGXL_LIBRARY

# to remove @x at the end of symbols
QMAKE_LFLAGS += --enable-stdcall-fixup -Wl,gtl-igxl.def

SOURCES += libgtligxl.cpp

HEADERS += libgtligxl.h \
        libgtl-igxl_global.h

# MinGW windres is given an error (windres: preprocessing failed) without any root cause
# Make sure the cybwin windres is used (check PATH or remove MinGW windres.exe)
RC_FILE = libgtl-igxl.rc

INCLUDEPATH += .
INCLUDEPATH += ../../../gtl/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

CONFIG(debug, debug|release) {
  LIBS += libgtl_cored libgstdld
  #PRE_TARGETDEPS += ../lib/$$OSFOLDER/libgtl_cored.a
}

PRE_TARGETDEPS += ../../../gtl/lib/$$OSFOLDER/lib$$gexTargetName(gtl_core).a

CONFIG(release, debug|release) {
  LIBS += libgtl_core libgstdl
}

LIBS += -L../../../gtl/lib/win32
LIBS += -L../../../../../../galaxy_libraries/galaxy_std_libraries/lib/win32

DESTDIR = ..

# For a xls file to use a function in a dll, the dll needs to be here...
# This is proviking such error:
# cp -f "../gtl-igxl.dll" ../../../../../../../../../../../Windows/System32
# cp: impossible de créer le fichier standard "../../../../../../../../../../../Windows/System32": No such file or directory
# Makefile_win32.Release:77: recipe for target "../gtl-igxl.dll" failed
# mingw32-make[1]: [../gtl-igxl.dll] Error 1 (ignored)
# cp: impossible d'evaluer "C:/cygwin/home/gexprod/prod/gex-prod-master/galaxy_products/gex_product/gex-tester/gtl/lib/win32/libgtl.a": No such file or directory
# DLLDESTDIR = c:/Windows/System32

INCLUDEPATH += $(DEVDIR)/other_libraries/syslog-win32/include
# shouls prevent the need for 2 dlls...
LIBS += -static-libstdc++ -static-libgcc -static -lpthread
LIBS += libwsock32 libws2_32 libole32 liboleaut32 libsnmpapi
# win32:DEFINES += -mwindows
win32:QMAKE_LFLAGS += -shared

CONFIG(debug, debug|release) {
  QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* c:/Windows/System32
}

OTHER_FILES += libgtl-igxl.rc
