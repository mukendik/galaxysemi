TEMPLATE = lib
TARGET = gs_data
QT -= core gui

win32-g++ {
  TARGET = lib$$TARGET
  CONFIG += dll
  DEFINES -= UNICODE
  QMAKE_CFLAGS += -posix
}

include($(DEVDIR)/galaxy_common.pri)
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

CONFIG += debug_and_release

SOURCES =\
 gs_data.c\
 gs_buffer.c\

HEADERS =\
 ../include/gs_data.h\
 ../include/gs_buffer.h\
 dlfcn_win32.h\

INCLUDEPATH += ../include
