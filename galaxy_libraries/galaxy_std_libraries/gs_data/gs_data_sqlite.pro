TEMPLATE = lib
TARGET = gs_data_sqlite
QT -= core gui

win32-g++ {
  TARGET = lib$$TARGET
  CONFIG += dll
  DEFINES -= UNICODE
  QMAKE_CFLAGS += -posix
}

include($(DEVDIR)/galaxy_common.pri)
DESTDIR = $(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
DLLDESTDIR = $(PWD)/../../../bin/plugins/db

CONFIG += debug_and_release

SOURCES =\
 gs_data.c\
 gs_data_sqlite.c\
 gs_buffer.c\

HEADERS =\
 ../include/gs_data.h\
 ../include/gs_buffer.h

INCLUDEPATH += ../include

win32-g++ {
  INCLUDEPATH += ../../../other_libraries/sqlite/3.7.17
  SOURCES += ../../../other_libraries/sqlite/3.7.17/sqlite3.c
  QMAKE_CFLAGS_WARN_ON += -Wno-error
}
unix {
  LIBS += -lsqlite3
}

unix|linux-g++-* {
  QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET*\
    $(DEVDIR)/galaxy_products/gex_product/bin/plugins/db
}
