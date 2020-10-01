TARGET = galaxy_ut_pat_recipe

CONFIG += debug_and_release

QT -= gui
QT -= opengl

include($(DEVDIR)/galaxy_common.pri)

LIBS += -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gexdb_plugin_base)

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include

SOURCES += main.cpp 

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl_patcore)
LIBS += -l$$gexTargetName(gqtl_stdf)
LIBS += -l$$gexTargetName(gqtl)
