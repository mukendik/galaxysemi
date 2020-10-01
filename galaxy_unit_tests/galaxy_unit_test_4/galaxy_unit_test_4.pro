TEMPLATE = app
CONFIG += console
CONFIG += qt
QT += widgets
QT += xml
QT += network
QT += script
QT += sql
QT += scripttools

include($(DEVDIR)/galaxy_common.pri)

PRE_TARGETDEPS += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER/lib$$gexTargetName(gstdl).a

TARGET = ut

# EProfiler
#QMAKE_CXXFLAGS_DEBUG += -finstrument-functions
#QMAKE_CFLAGS_DEBUG += -finstrument-functions
#QMAKE_LFLAGS += -Wl,-Map=unit_test_3.map
#win32: LIBS += -L$(DEVDIR)/other_libraries/EProfiler/lib/win32 -lEProfiler.dll

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources/pat
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources/wafer
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources/ui
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/include
INCLUDEPATH += $(QTSRCDIR)
INCLUDEPATH += $(DEVDIR)/other_libraries/chartdirector/include
INCLUDEPATH += $(DEVDIR)
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/license-provider-plugin/base-lp/sources

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl)
LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER -l$$gexTargetName(base_lp)

# add these libs at last in order to link...
win32:LIBS += libwsock32 libws2_32 libole32 liboleaut32

SOURCES += main.cpp
SOURCES += $(DEVDIR)/galaxy_products/gex_product/gex/sources/mo_datapump.cpp

#message($$LIBS)
