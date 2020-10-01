QT -= core gui xml opengl
QT += network

TARGET = galaxy_unit_test_2
TEMPLATE = app

CONFIG += console
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include

SOURCES += main.cpp 

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
# LIBS += -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER

LIBS += -l$$gexTargetName(gstdl)

win32:LIBS += libwsock32 libws2_32
