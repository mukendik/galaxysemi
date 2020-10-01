QT -= core gui xml opengl
QT += network

TARGET = galaxy_unit_test_gs_type
TEMPLATE = app

CONFIG += console
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

SOURCES += main.cpp 


LIBS += $$GALAXY_LIBSPATH
LIBS += -l$$gexTargetName(gstdl)

