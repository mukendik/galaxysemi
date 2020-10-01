# -------------------------------------------------
# Project created by QtCreator 2009-05-05T16:59:17
# -------------------------------------------------
# put in user_custom.pri any custom defines...
exists(user_custom.pri) {
  include(user_custom.pri)
}

DEFINES += MODULE=GEXDB_PLUGIN_MEDTRONIC
#
QT += network \
  sql \
  xml \
  xml \
  widgets

#needed for GexScriptEngine
QT += script

TEMPLATE = lib
DEFINES += GEXDB_PLUGIN_EXPORTS
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)
#
DESTDIR		= $(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
# strangely, $(DEVDIR) in DLLDESTDIR is unusable on Win7 because all '\' have been removed. Let s use $$PWD
#DLLDESTDIR	= $(DEVDIR)/galaxy_products/gex_product/bin/plugins
#DLLDESTDIR	= $(PWD)/../../bin/plugins
DLLDESTDIR	= $(PWD)/../../../bin/plugins/db
#
TARGET = $$gexTargetName(gexdb_plugin_medtronic)
#
#RC_FILE = gexdb_plugin_medtronic_resource.rc
#

SOURCES += sources/gexdb_plugin_medtronic_rdb_optionswidget.cpp \
    sources/gexdb_plugin_medtronic_options.cpp \
    sources/gexdb_plugin_medtronic_cfgwizard.cpp \
    sources/gexdb_plugin_medtronic.cpp
HEADERS += sources/gexdb_plugin_medtronic_rdb_optionswidget.h \
    sources/gexdb_plugin_medtronic_options.h \
    sources/gexdb_plugin_medtronic_cfgwizard.h \
    sources/gexdb_plugin_medtronic.h
FORMS += sources/gexdb_plugin_medtronic_rdb_optionswidget.ui \
    sources/gexdb_plugin_medtronic_cfg_wizard.ui
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
  $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources/ui \
  $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_medtronic/sources/ui \
  $(QTSRCDIR)
#  $(DEVDIR)/galaxy_products/gex_product/gex-log/include
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
  $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
  $(DEVDIR)/galaxy_products/gex_product/include \
  $(DEVDIR)/galaxy_products/gex_product/plugins/include \
  $(DEVDIR)/galaxy_products/gex_product/gex/sources \
  $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources \
  $$PWD/sources
equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,2){
    UI_DIR = $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_medtronic/sources/ui
}

GALAXY_LIBS +=\
 -l$$gexTargetName(gqtl_stdf)\
 -l$$gexTargetName(gexdb_plugin_base)\
 -l$$gexTargetName(gstdl)

# path to static libs (.a) : plugin base, plugin galaxy, ....
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER \
  -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \
  -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER

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

win32:LIBS += -lws2_32

linux*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

unix {
  QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/plugins/db
}
