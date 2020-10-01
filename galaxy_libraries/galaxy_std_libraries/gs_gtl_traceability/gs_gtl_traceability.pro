TEMPLATE = lib
TARGET = gs_gtl_traceability
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
 gs_gtl_traceability.c\
 gs_json.c\

HEADERS =\
 ../include/gs_gtl_traceability.h\
 ../include/gs_json.h\
 ../include/gs_data.h\
 ../include/gs_buffer.h\

INCLUDEPATH += ../include

LIBS +=\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -lgs_data\

win32-g++ {
  INCLUDEPATH += ../../../other_libraries/sqlite/3.7.17
  SOURCES += ../../../other_libraries/sqlite/3.7.17/sqlite3.c
  QMAKE_CFLAGS_WARN_ON += -Wno-error
}
unix {
  LIBS += -lsqlite3
}
