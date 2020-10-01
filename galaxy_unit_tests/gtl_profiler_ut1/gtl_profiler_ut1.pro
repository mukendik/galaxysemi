#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T13:21:19
#
#-------------------------------------------------

QT       -= core
QT       -= gui

include($(DEVDIR)/galaxy_common.pri)

TARGET = gtl_profiler_ut1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CFLAGS += -std=gnu99
# c99 is complaining about timespec. Lets try gnu99
QMAKE_CFLAGS_DEBUG += -std=gnu99

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include
DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include
LIBS += -l$$gexTargetName(gtl_core) -l$$gexTargetName(gstdl)
LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
LIBS += -L$$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

win32: LIBS += -lws2_32 -lsnmpapi
linux: LIBS += -lpthread -lrt -ldl

SOURCES += main.c

win32-g++: QMAKE_CFLAGS += -posix
