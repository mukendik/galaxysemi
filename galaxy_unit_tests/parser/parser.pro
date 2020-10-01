# ---------------------------------------------------------------------------- #
# parser.pro
# ---------------------------------------------------------------------------- #
TARGET = parser

CONFIG += debug_and_release

QT -= gui
QT -= opengl

SOURCES += import_verigy_edl.cpp main.cpp dl4_tools.cpp

include($(DEVDIR)/galaxy_common.pri)

LIBS += -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gexdb_plugin_base)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH +=\
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl_parser)
LIBS += -l$$gexTargetName(gqtl_stdf)
LIBS += -l$$gexTargetName(gqtl)

LIBS += -l$$gexTargetName(fnp_lp)
LIBS += -l$$gexTargetName(base_lp)

linux-g++-64: LIBS += -ldl -lrt
