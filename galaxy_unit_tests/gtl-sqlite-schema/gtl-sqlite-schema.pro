# ---------------------------------------------------------------------------- #
# gtl-sqlite-schema.pro
# ---------------------------------------------------------------------------- #
TARGET = gtl-sqlite-schema

CONFIG += debug_and_release

QT -= core gui opengl

SOURCES += main.cpp

include($(DEVDIR)/galaxy_common.pri)
INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/sqlite
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
LIBS += -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gtl)

linux-g++-64: LIBS += -ldl -lrt

win32-g++ {
    QMAKE_LIBS_QT_ENTRY -= -lqtmain
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gtl_core)
    LIBS += -l$$gexTargetName(gstdl)
    LIBS += -lws2_32 -lsnmpapi
}
