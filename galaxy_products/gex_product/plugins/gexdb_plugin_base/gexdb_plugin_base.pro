# -------------------------------------------------
# Project created by QtCreator 2009-03-30T11:17:52
# -------------------------------------------------
# user specific cusom defines,... just put a user_custom.pri file in the project folder and it will appears in QtCreator
exists(user_custom.pri) {
  include(user_custom.pri)
}
#
DEFINES += MODULE=GEXDB_PLUGIN_BASE

#
QT += network \
    sql \
    xml

#needed by GexScriptEngine
QT += script

#needed for QT5
QT += widgets

TEMPLATE = lib
CONFIG += debug_and_release

# Warnings
QMAKE_CFLAGS_WARN_ON    = -w  # suppr warnings from sqlite3.c

include($(DEVDIR)/galaxy_common.pri)
#include($$PWD/db_keys.pri)
include($$PWD/db_architecture.pri)
include($$PWD/query_engine.pri)
include($$PWD/query_progress.pri)

DESTDIR = $$PWD/../lib/$$OSFOLDER
TARGET = $$gexTargetName(gexdb_plugin_base)

SOURCES += \
# sources/gexdb_plugin_itemselectiondialog.cpp \
    $(QTSRCDIR)/qtbase/src/3rdparty/sqlite/sqlite3.c \
    sources/gexdb_plugin_querybuilder.cpp \
    sources/gexdb_plugin_er_dataset.cpp \
    sources/gexdb_plugin_er_parts.cpp \
    sources/gexdb_plugin_er_parts_seriedef.cpp \
    sources/gexdb_plugin_itemselectiondialog.cpp \
    sources/gexdb_plugin_connector.cpp \
    sources/gexdb_plugin_bin_info.cpp \
    sources/gexdb_plugin_filter.cpp \
    sources/gexdb_plugin_query.cpp \
    sources/gexdb_plugin_bin_list.cpp \
    sources/gexdb_plugin_mapping_field.cpp\
    sources/gexdb_plugin_base.cpp \
    sources/gexdb_plugin_filetransfer_dialog.cpp \
    sources/xychart_data.cpp \
    sources/gexdb_plugin_monitordatapoint.cpp \
    sources/gexdb_plugin_monitorstat.cpp
#
HEADERS += ../include/gexdb_plugin_base.h \
    ../include/gexdb_plugin_datafile.h \
    ../include/gexdb_plugin_common.h \
  # ../include/gexdb_plugin_itemselectiondialog.h \
    ../include/gexdb_plugin_filetransfer_dialog.h \
    ../include/gexdb_plugin_querybuilder.h \
    ../include/gexdb_plugin_er_dataset.h \
    ../include/gexdb_plugin_itemselectiondialog.h \
    ../include/gexdb_plugin_sya.h \
    ../include/gexdb_plugin_bin_info.h \
    ../include/gexdb_plugin_option.h \
    ../include/xychart_data.h \
    ../include/gexdb_plugin_monitordatapoint.h \
    ../include/gexdb_plugin_monitorstat.h \
    ../include/statistical_monitoring_datapoint_struct.h \
    ../include/statistical_monitoring_alarm_struct.h \
    ../include/statistical_monitoring_limit_struct.h \
    ../include/statistical_monitoring_item_desc.h \
    ../include/statistical_monitoring_monitored_item_unique_key_rule.h

FORMS += sources/gexdb_plugin_itemselection_dialog.ui \
    sources/gexdb_plugin_filetransfer_dialog.ui
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/ui \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources/ui \
    $(QTSRCDIR)
#    $(DEVDIR)/galaxy_products/gex_product/gex-log/include


INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/license-provider-plugin/base-lp/sources

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $$PWD/sources

equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,2){
    UI_DIR = $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources/ui
}

GALAXY_LIBS = -l$$gexTargetName(gqtl_ftp) \
-l$$gexTargetName(gstdl) \
    -l$$gexTargetName(gqtl)
#	-l$$gexTargetName(gqtl_sysutils) \

GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER \
  -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH

win32:LIBS += -lws2_32 -lkernel32  -lpsapi
linux*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

