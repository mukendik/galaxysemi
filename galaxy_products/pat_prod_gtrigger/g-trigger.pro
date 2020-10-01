DEFINES += MODULE=GTRIGGER

QT += xml
QT += widgets

TEMPLATE = app
CONFIG += debug_and_release
CONFIG -= app_bundle

include($(DEVDIR)/galaxy_products/pat_prod_gtrigger/testable_g_trigger.pri)

DESTDIR = $(DEVDIR)/galaxy_products/pat_prod_gtrigger/bin
TARGET = $$gexTargetName(g-trigger)

SOURCES += \
    sources/profile.cpp \
    sources/g-trigger_dialog.cpp \
    sources/main.cpp \
    sources/fettest_nopat.cpp \
    sources/cstdf.cpp \
    sources/g-trigger.cpp \
    sources/g-trigger_engine.cpp\
    sources/g-trigger_engine_fet_test.cpp\
    sources/g-trigger_engine_eagle.cpp\
    sources/g-trigger_engine_spektra.cpp\
    sources/g-trigger_engine_create_trigger_files.cpp\

HEADERS += \
    sources/profile.h \
    sources/g-trigger.h \
    sources/cstdf.h \
    sources/g-trigger.h \
    sources/g-trigger_dialog.h \
    ../gex_product/gex/sources/converter_external_file.h \
    sources/g-trigger_engine.h
FORMS += sources/g-trigger_dialogbase.ui
INCLUDEPATH = \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/pat_prod_gtrigger/sources/ui

INCLUDEPATH += \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft

DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/include \
  $$PWD/sources

RESOURCES += \
    sources/g-trigger.qrc

OBJECTS_DIR = $$G_TRIGGER_OBJECTS_DIR

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/other_libraries/sqlite/3.7.17

LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl_patcore)
LIBS += -l$$gexTargetName(gqtl)
LIBS += -l$$gexTargetName(gqtl_parser)
LIBS += -l$$gexTargetName(gexdb_plugin_base)
LIBS += -lsqlite

LIBS += -l$$gexTargetName(qx_std_utils)
LIBS += -l$$gexTargetName(qx_bin_mapping)

linux-g++* {
    LIBS += -ldl
}

#PLEASE DON T REMOVE THIS LINE
#user_custom.pri must be used to customize the .pro in your environment.
#Use this in order to do tests which won t be committed on the SCM.
exists(user_custom.pri) {
  include(user_custom.pri)
}

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

OTHER_FILES += \
    buildpackage/windows_scripts/g-trigger.iss \
    sources/html/g-trigger-history.htm

win32:LIBS += -lPsapi

DISTFILES += \
    buildpackage/windows_scripts/g_trigger_v35-win32.iss \
    buildpackage/windows_scripts/g-trigger-bg.iss \
    buildpackage/windows_scripts/g_trigger_v37-win32.iss \
    buildpackage/windows_scripts/g_trigger-win32.iss
