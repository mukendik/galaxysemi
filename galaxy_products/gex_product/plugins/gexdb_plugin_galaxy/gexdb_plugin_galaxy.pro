# -------------------------------------------------
# Project created by QtCreator 2009-05-06T18:06:09
# -------------------------------------------------
# put in user_custom.pri any custom defines...
exists(user_custom.pri) {
    include(user_custom.pri)
}

DEFINES += MODULE=GEXDB_PLUGIN_GALAXY

#
QT += network \
    sql \
    xml \
    xmlpatterns \
    widgets

# needed for GexScriptEngine
QT += script

# webkit necessary for svg waferzones files
QT +=	webkit

solaris-g++:CONFIG += gex_no_webkit

TEMPLATE = lib
DEFINES += GEXDB_PLUGIN_EXPORTS
#DEFINES += QT3_SUPPORT
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)
include(consolidation_tree.pri)
#
DESTDIR		= $(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
DLLDESTDIR	= $(PWD)/../../../bin/plugins/db

TARGET = $$gexTargetName(gexdb_plugin_galaxy)
#
RC_FILE = gexdb_plugin_galaxy_resource.rc
#
SOURCES += \
    sources/gexdb_plugin_galaxy_insert_sqlitefile.cpp \
    sources/gexdb_plugin_galaxy_insert_wyr.cpp \
    sources/gexdb_plugin_galaxy_insert.cpp \
    sources/gexdb_plugin_galaxy_extract_er.cpp \
    sources/gexdb_plugin_galaxy_extract.cpp \
    sources/gexdb_plugin_galaxy_cfgwizard.cpp \
    sources/gexdb_plugin_galaxy_admin.cpp \
    sources/gexdb_plugin_galaxy.cpp \
    sources/gexdb_plugin_galaxy_insert_sqlite.cpp \
    sources/gexdb_plugin_galaxy_extract_consolidated.cpp \
    sources/gexdb_plugin_galaxy_create_stdf.cpp \
    sources/gexdb_plugin_galaxy_export_to_stdf.cpp \
    sources/gexdb_plugin_galaxy_settings.cpp \
    sources/gexdb_getroot_dialog.cpp \
    sources/gexdb_plugin_galaxy_admin_b12.cpp \
    sources/gexdb_plugin_galaxy_admin_consolidated.cpp \
    sources/gexdb_plugin_galaxy_admin_to_innodb.cpp \
    sources/gexdb_plugin_galaxy_admin_consolidated_az.cpp \
    sources/gexdb_plugin_galaxy_write.cpp \
    sources/gexdb_plugin_galaxy_querydatafiles.cpp \
    sources/gexdb_plugin_galaxy_query.cpp \
    sources/gexdb_plugin_galaxy_querybin.cpp \
    sources/gexdb_plugin_galaxy_queryproduct.cpp \
    sources/gexdb_plugin_galaxy_construct_splitlot_query.cpp \
    sources/gexdb_plugin_galaxy_queryfield.cpp \
    sources/gexdb_plugin_galaxy_construct_query.cpp \
    sources/gexdb_plugin_galaxy_querydatafiles_et_perdie.cpp \
    sources/rdb_options_widget.cpp \
    sources/rdb_options.cpp \
    sources/gexdb_plugin_galaxy_get.cpp \
    sources/gexdb_plugin_galaxy_init_maps.cpp \
    sources/gexdb_plugin_galaxy_init_maps_et_perdie.cpp \
    sources/gexdb_plugin_galaxy_create_stdf_et_perdie.cpp \
    sources/libgexsvg.cpp \
    sources/gexdb_plugin_galaxy_admin_b13.cpp \
    sources/gexdb_plugin_galaxy_fill.cpp \
    sources/gexdb_plugin_galaxy_splitlotlist.cpp \
    sources/gexdb_plugin_galaxy_admin_b14.cpp \
    sources/gexdb_plugin_galaxy_testfilter.cpp \
    sources/gexdb_plugin_galaxy_admin_b15.cpp \
    sources/gexdb_plugin_galaxy_consolidation_gui.cpp \
    sources/gexdb_plugin_galaxy_consolidation_tree.cpp \
    sources/gexdb_plugin_galaxy_admin_b16.cpp \
    sources/gexdb_global_files.cpp \
    sources/gexdb_plugin_galaxy_admin_b17.cpp \
    sources/xmlsyntaxhighlighter.cpp \
    sources/gexdb_plugin_galaxy_insert_update.cpp \
    sources/gexdb_plugin_galaxy_admin_b18.cpp\
    sources/gexdb_plugin_galaxy_admin_b19.cpp\
    sources/gexdbthreadquery.cpp \
    sources/gexdb_plugin_galaxy_admin_b20.cpp \
    sources/gexdb_plugin_galaxy_extract_with_dietrace.cpp \
    sources/gexdb_plugin_galaxy_option.cpp \
    sources/gexdb_plugin_galaxy_admin_b21.cpp \
    sources/gexdb_plugin_galaxy_admin_b22.cpp \
    sources/gexdb_plugin_galaxy_admin_b23.cpp \
    sources/gexdb_plugin_galaxy_admin_b24.cpp \
    sources/gexdb_plugin_galaxy_admin_b25.cpp \
    sources/gexdb_plugin_galaxy_spm.cpp \
    sources/gexdb_plugin_galaxy_csv_condition.cpp \
    sources/gexdb_plugin_galaxy_create.cpp \
    sources/gexdb_plugin_galaxy_consolidation.cpp \
    sources/gexdb_plugin_galaxy_sya.cpp

# No webkit in Solaris so no genealogy
gex_no_webkit {
    QT -= webkit
    DEFINES += GEX_NO_WEBKIT
    # SOURCES -= sources/gexdb_plugin_galaxy_querydatafiles_et_perdie.cpp
}

HEADERS += sources/gexdb_plugin_galaxy_cfgwizard.h \
    sources/gexdb_plugin_galaxy.h \
    sources/gexdb_plugin_galaxy_defines.h \
    sources/gexdb_getroot_dialog.h \
    sources/rdb_options_widget.h \
    sources/rdb_options.h \
    sources/consolidation_center.h \
    sources/gexdb_global_files.h \
    sources/xmlsyntaxhighlighter.h \
    sources/gexdb_plugin_galaxy_splitlot_info.h \
    sources/test_filter.h \
    sources/gexdbthreadquery.h \
    sources/tdr_gs_version.h \
    sources/gexdb_plugin_galaxy_database_settings.h \
    include/statistical_monitoring_tables.h

FORMS += sources/gexdb_plugin_galaxy_cfg_wizard.ui \
    sources/gexdb_getroot_dialog.ui \
    sources/rdb_options_widget.ui

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/include \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/ui \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_base/sources/ui \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/sources/ui \
    $(DEVDIR)/galaxy_products/gex_product/gex-pb/include \
    $(QTSRCDIR) \
    $(DEVDIR)/other_libraries/sqlite/3.7.17 \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/common \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/statistical_agents \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/external_services/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/external_services/sources/statistical_agents \

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/common \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/statistical_agents \
    #$(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/sources/ui \
    $$PWD/sources

equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,2){
    UI_DIR = $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/sources/ui
}

GALAXY_LIBS = \
-lgs_data \
-lgs_gtl_traceability \
-l$$gexTargetName(gstdl) \
-l$$gexTargetName(gexdb_plugin_base) \
-l$$gexTargetName(gqtl_stdf) \
-l$$gexTargetName(gqtl_ftp) \
#    -l$$gexTargetName(gstdl_utils) \
#    -l$$gexTargetName(gstdl_errormgr) \
#    -l$$gexTargetName(gstdl_blowfish_c) \
    -l$$gexTargetName(gqtl) \
#    -l$$gexTargetName(gqtl_sysutils) \
#    -l$$gexTargetName(gqtl_utils) \
    -l$$gexTargetName(gqtl_svg) \
    -l$$gexTargetName(gexpb) \
    -l$$gexTargetName(gqtl_datakeys)

# path to static libs (.a) : plugin base, plugin galaxy, ....
GALAXY_LIBSPATH =\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/gex-pb/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/bin\
 -L$(DEVDIR)/galaxy_products/gex_product/gex/external_services/lib/$$OSFOLDER\
 -lexternal_services

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS +=\
 $$GALAXY_LIBSPATH\
 $$GALAXY_LIBS\
 -l$$gexTargetName(qx_std_utils)

# Psapi is now needed for CGexSystemUtils::GetMemoryInfo
win32:LIBS += -lws2_32 -lPsapi

linux*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

OTHER_FILES += gexdb_plugin_galaxy_resource.rc \
              sources/resources/galaxy_tables_dependencies.xml

unix {
    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/plugins/db
}

RESOURCES += \
    sources/gexdb_plugin_galaxy.qrc

# user specific defines...
exists(user_custom.pri) {
    include(user_custom.pri)
}
