QT -= core gui xml opengl
QT += network
# QT += core thread

# gprof ?
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_CXXFLAGS += -gstabs+ -pg
#QMAKE_CXXFLAGS_DEBUG += -gstabs+ -pg
#QMAKE_CFLAGS += -gstabs+ -pg
#QMAKE_CFLAGS_DEBUG += -gstabs+ -pg
#QMAKE_LFLAGS += -pg
#QMAKE_STRIP =
#QMAKE_STRIP = $${CROSS_COMPILE}strip
#QMAKE_STRIPFLAGS_LIB += --strip-unneeded

include($(DEVDIR)/galaxy_common.pri)

TARGET = galaxy_unit_test_1
contains(DEFINES, GTL_PROFILER_OFF): TARGET = galaxy_unit_test_1_noprof

TEMPLATE = app

CONFIG += console
CONFIG += debug_and_release
# CONFIG += threads

# Let's build in dynamic ? do we have path to GTL ?
DEFINES += DYNGTL

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include

SOURCES += main.cpp

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
#
contains(DEFINES, DYNGTL){
  # dyn libs : gtl.a and gtl.dll/so/dylib
  contains(DEFINES, GTL_PROFILER_OFF){
    LIBS += -l$$gexTargetName(gtl_noprof)
    WIN32: PRE_TARGETDEPS += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/$$gexTargetName(gtl_noprof).dll
  }
  else{
  LIBS += -l$$gexTargetName(gtl)
  WIN32: PRE_TARGETDEPS += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/$$gexTargetName(gtl).dll
}
}
else {
  # static lib : gtl_core.a & gstdl
  contains(DEFINES, GTL_PROFILER_OFF): LIBS +=  -l$$gexTargetName(gtl_core_noprof)
  !contains(DEFINES, GTL_PROFILER_OFF): LIBS +=  -l$$gexTargetName(gtl_core)
  LIBS += -l$$gexTargetName(gstdl)
}

linux-g++-64: LIBS += -ldl -lrt

message($$CONFIG $$LIBS)

HEADERS += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include/gtl_core.h

win32:LIBS += libwsock32 libws2_32 -lPsapi -lsnmpapi

OTHER_FILES += \
    unit_test_recipe.csv

# EProfile ?
#QMAKE_CXXFLAGS_DEBUG += -finstrument-functions
#QMAKE_CFLAGS_DEBUG += -finstrument-functions
#QMAKE_LFLAGS += -Wl,-Map=ProjectName.map
#win32: LIBS += -L$(DEVDIR)/other_libraries/EProfiler/lib/win32 -lEProfiler.dll
