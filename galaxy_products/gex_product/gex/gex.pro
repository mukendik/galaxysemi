# -------------------------------------------------
DEFINES += MODULE=GEX
DEFINES+=OUTLIER_REMOVAL

# LTXC package with LTX restrictions
gex_oem_ltxc: DEFINES+=OEM_LTXC

# Teradyne package
oemTeradyne: DEFINES+=TER_GEX

daemon: DEFINES+=GSDAEMON

# PAT Build
pat_prod: DEFINES+=GCORE15334

# Activate performance logs for WS PAT
bench_ws_pat: DEFINES+=WS_PAT_PERFORMANCE

# user specific defines...
exists(user_custom.pri) {
    include(user_custom.pri)
}
# Qt modules
QT += network \
    sql \
    xml \
    webkit \
    script
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
DEFINES += QT_HASH_SEED=1
QT += printsupport
QT += webkitwidgets
QT += printsupport

QT += opengl

# Now needed by JavaScriptCenter GUI
QT += scripttools

TEMPLATE = app
CONFIG += threads
CONFIG += debug_and_release

contains(DEFINES, GSDAEMON):message(Building daemon...)

# Force calcul unit to SSE
linux-g++-32:QMAKE_CXXFLAGS += -mfpmath=sse -march=pentium4
! linux-g++-64:message(QMAKE_COMPILER_DEFINES : $$QMAKE_COMPILER_DEFINES )
win32:{
  !contains(QMAKE_HOST.arch, x86_64) {
  QMAKE_CXXFLAGS += -mfpmath=sse -march=pentium4
  }
}
! linux-g++-64:message(QMAKE_CXXFLAGS : $$QMAKE_CXXFLAGS )

solaris-g++:CONFIG += gex_no_webkit gex_no_js_solaris

include($(DEVDIR)/galaxy_common.pri)
! linux-g++-64:message(OS folder : $$OSFOLDER)

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin

contains(DEFINES, GSDAEMON): TARGET = $$gexTargetName(gsd)
!contains(DEFINES, GSDAEMON): TARGET = $$gexTargetName(gex)

! linux-g++-64:message(TARGET : $$TARGET)

include(database.pri)
include(csl.pri)
include(export.pri)
include(browser.pri)
include(import.pri)
include(sources/monitoring/monitoring.pri)
include(reports_center.pri)
include(interactive_table.pri)
include(pat.pri)
include(patjson.pri)
include(settings_dialog.pri)
include(spektra.pri)
include(enterprise_report.pri)
include(gex_report.pri)
include(advanced_enterprise_report.pri)
include(options.pri)
include(wafer.pri)
include(gtm.pri)
include(interactive_charts.pri)
include(common_widgets.pri)
include(detachable_tab_window_container.pri)
include(on_the_fly_report.pri)

SOURCES += \
sources/admin_engine_update.cpp \
    sources/max_shift_calculator.cpp \
sources/pickuser_dialog.cpp \
  sources/pick_fields_dialog.cpp \
  sources/picktest_dialog.cpp \
  sources/pickserie_dialog.cpp \
  sources/pickproduct_id_dialog.cpp \
  sources/pickname_dialog.cpp \
  sources/pickfilter_dialog.cpp \
  sources/pickcoord_dialog.cpp \
  sources/pickcombo_dialog.cpp \
  sources/pick_parameterfilter_dialog.cpp \
  sources/pick_export_wafermap_dialog.cpp \
  sources/pick_audit_filter_dialog.cpp \
  sources/pickpart_dialog.cpp \
  sources/picktest_item.cpp \
    sources/gs_qa_dump.cpp \
    sources/subset_limits.cpp \
    sources/pat_report_ft_gui.cpp \
    sources/test_multi_result_item.cpp \
    sources/bin_description.cpp \
    sources/test_marker.cpp \
    sources/common_widgets/multi_select_input_dialog.cpp \
    sources/wafermap_export.cpp

SOURCES += \
    sources/temporary_files_manager.cpp \
    sources/message.cpp \
    sources/report_template.cpp \
    sources/report_template_io.cpp \
    sources/report_template_gui.cpp \
    sources/timeperiod_dialog.cpp \
    sources/stdf_record_dialog.cpp \
    sources/spektra_record_dialog.cpp \
    sources/snapshot_dialog.cpp \
    sources/scripting_io.cpp \
    sources/script_wizard.cpp \
    sources/report_readfile.cpp \
    sources/report_frontpage_dialog.cpp \
    sources/report_classes_sorting.cpp \
    sources/processoutput_dialog.cpp \
    sources/process_launcher.cpp \
    sources/onefile_wizard.cpp \
    sources/navigation_bars.cpp \
    sources/mixfiles_wizard.cpp \
    sources/mergefiles_wizard.cpp \
    sources/main.cpp \
    sources/hostid_snmpapi.cpp \
    sources/hostid_mib_ccess.cpp \
    sources/gextestmapping.cpp \
    sources/gextestfilter.cpp \
    sources/gexinputdialog.cpp \
    sources/gexdatasetconfigio.cpp \
    sources/gexdatasetconfig.cpp \
    sources/gex_word_report.cpp \
    sources/gex_web.cpp \
    sources/gex_ppt_report.cpp \
    sources/gex_pdf_report.cpp \
    sources/gex_debug_dialog.cpp \
    sources/gex_debug.cpp \
    sources/gex_constants.cpp \
    sources/getstring_dialog.cpp \
    sources/gcolorpushbutton.cpp \
    sources/filter_dialog.cpp \
    sources/pickbin_dialog.cpp \
    sources/pickbin_single_dialog.cpp \
    sources/pickproduct_idsql_dialog.cpp \
    sources/eval_exp_variant.cpp \
    sources/eval_exp_cexev.cpp \
    sources/drill_what_if.cpp \
    sources/dl4_tools.cpp \
    sources/dl4_db.cpp \
    sources/ctest.cpp \
    sources/cstats.cpp \
    sources/conversionprogress_dialog.cpp \
    sources/comparefiles_wizard.cpp \
    sources/classes.cpp \
    sources/chtmlpng.cpp \
    sources/calendar_dialog.cpp \
    sources/bincolors_dialog.cpp \
    sources/assistant_wizard.cpp \
    sources/treewidgetitemcmp.cpp \
    sources/gexpartfilter.cpp \
    sources/colors_generator.cpp \
    sources/auto_repair_dialog.cpp \
    sources/auto_repair_stdf.cpp \
    sources/gex_test_to_create.cpp \
    sources/gex_test_creator.cpp \
    sources/test_result.cpp \
    sources/test_result_item.cpp \
    sources/gex_algorithms.cpp \
    sources/report_page_adv_multichart.cpp \
    sources/DebugMemory.cpp \
    sources/gexperformancecounter.cpp \
    sources/test_list_pearson.cpp \
    sources/gexabstractdatalog.cpp \
    sources/part_binning.cpp \
    sources/gex_file_in_group.cpp \
    sources/gex_group_of_files.cpp \
    sources/cmerged_results.cpp \
    sources/gex_box_plot_data.cpp \
    sources/gex_file_in_group_read.cpp \
    sources/cpart_info.cpp \
    sources/javascriptcenter.cpp \
    sources/qscriptsyntaxhighlighter.cpp \
    sources/customdialogbox.cpp \
    sources/gex_test_to_update.cpp \
    sources/gextestfilterprivate.cpp \
    sources/histogramdata.cpp \
    sources/cbinning.cpp \
    sources/command_line_options.cpp \
    sources/user_input_filter.cpp \
    sources/plugin_base.cpp \
    sources/plugin_manager.cpp \
    sources/plugin_eventdump_dialog.cpp \
    sources/daemon.cpp \
    sources/print_dialog.cpp \
    sources/report_build.cpp \
    sources/gex_undo_command.cpp \
    sources/ym_event_log_gui.cpp

include(toolbox.pri)
include(engine.pri)
include(interactive_wafermap.pri)
HEADERS += \
    sources/max_shift_calculator.h \
    sources/report_template.h \
    sources/report_template_gui.h \
    sources/timeperiod_dialog.h \
    sources/stdf_record_dialog.h \
    sources/spektra_record_dialog.h \
    sources/snapshot_dialog.h \
    sources/settings_sql.h \
    sources/scripting_io.h \
    sources/script_wizard.h \
    sources/resource.h \
    sources/report_frontpage_dialog.h \
    sources/report_classes_sorting.h \
    sources/report_build.h \
    sources/temporary_files_manager.h \
    sources/processoutput_dialog.h \
    sources/process_launcher.h \
    sources/print_dialog.h \
    sources/pickuser_dialog.h \
    sources/picktest_dialog.h \
    sources/pickserie_dialog.h \
    sources/pickproduct_id_dialog.h \
    sources/pickname_dialog.h \
    sources/filter_dialog.h \
    sources/pickfilter_dialog.h \
    sources/pickcoord_dialog.h \
    sources/pickcombo_dialog.h \
    sources/pick_parameterfilter_dialog.h \
    sources/pick_export_wafermap_dialog.h \
    sources/pick_audit_filter_dialog.h \
    sources/onefile_wizard.h \
    sources/mixfiles_wizard.h \
    sources/mergefiles_wizard.h \
    sources/iptypes.h \
    sources/gextestmapping.h \
    sources/gextestfilter.h \
    sources/gexinputdialog.h \
    sources/gexdatasetconfigio.h \
    sources/gexdatasetconfig.h \
    sources/gex_word_report.h \
    sources/gex_web.h \
    sources/gex_skins.h \
    sources/gex_ppt_report.h \
    sources/gex_pixmap_extern.h \
    sources/gex_pixmap.h \
    sources/gex_pdf_report.h \
    sources/gex_oem_constants.h \
    sources/gex_masa_mde.h \
    sources/gex_debug_dialog.h \
    ../include/gex_constants.h \
    sources/getstring_dialog.h \
    sources/gcolorpushbutton.h \
    sources/pickbin_dialog.h \
    sources/pickbin_single_dialog.h \
    sources/pickproduct_idsql_dialog.h \
    sources/eval_exp_variant.h \
    sources/eval_exp_cexev.h \
    sources/drill_what_if.h \
    sources/dl4_tools.h \
    sources/dl4_db.h \
    sources/ctest_chart_options.h \
    sources/ctest.h \
    sources/cstats.h \
    sources/conversionprogress_dialog.h \
    sources/comparefiles_wizard.h \
    sources/compare_query_wizard.h \
    sources/classes.h \
    sources/chtmlpng.h \
    sources/calendar_dialog.h \
    sources/bincolors_dialog.h \
    sources/assistant_flying.h \
    sources/gexperformancecounter.h \
    ../plugins/include/gexdb_plugin_base.h \
    sources/treewidgetitemcmp.h \
    sources/gexpartfilter.h \
    sources/colors_generator.h \
    sources/pickpart_dialog.h \
    ../include/gex_shared.h \
    sources/auto_repair_dialog.h \
    sources/auto_repair_stdf.h \
    ../include/gex_version.h \
    sources/gex_test_to_create.h \
    sources/gex_test_creator.h \
    sources/test_result.h \
    sources/test_result_item.h \
    sources/gex_algorithms.h \
    sources/import_acco.h \
    sources/DebugMemory.h \
    ../include/gex_scriptengine.h \
    sources/test_list_pearson.h \
    sources/gexabstractdatalog.h \
    sources/gex_file_in_group.h \
    sources/cmir.h \
    sources/cpart_binning.h \
    sources/gex_group_of_files.h \
    sources/cmerged_results.h \
    sources/gex_site_limits.h \
    sources/cpart_info.h \
    sources/cbinning.h \
    sources/gex_box_plot_data.h \
    sources/bin_info.h \
    sources/file.h \
    sources/javascriptcenter.h \
    sources/customdialogbox.h \
    sources/picktest_item.h \
    sources/pick_fields_dialog.h \
    sources/gex_test_to_update.h \
    sources/waf_bin_mismatch.h \
    sources/gextestfilterprivate.h \
    sources/histogramdata.h \
    sources/command_line_options.h  \
    sources/user_input_filter.h \
    sources/plugin_base.H \
    sources/plugin_manager.h \
    sources/plugin_eventdump_dialog.h \
    sources/plugin_constants.h \
    sources/daemon.h \
    sources/gs_qa_dump.h \
    sources/subset_limits.h \
    sources/pat_report_ft_gui.h \
    sources/bin_description.h \
    sources/test_marker.h \
    sources/QTitleFor.h \
    sources/gex_undo_command.h \
    sources/ym_event_log_gui.h \
    sources/common_widgets/multi_select_input_dialog.h \
    sources/wafermap_export.h

RESOURCES += \
    sources/gex.qrc

FORMS += sources/wizard_custom_report_edit_dialog.ui \
    sources/wizard_custom_report_admin_dialog.ui \
    sources/timeperiod_dialog.ui \
    sources/suggestlimits_dialog.ui \
    sources/stdf_record_dialog.ui \
    sources/spektra_record_dialog.ui \
    sources/snapshot_dialog.ui \
    sources/script_dialog.ui \
    sources/report_frontpage_dialog.ui \
    sources/process_output_dialog.ui \
    sources/print_dialog.ui \
    sources/pickuser_dialog.ui \
    sources/picktest_dialog.ui \
    sources/pickserie_dialog.ui \
    sources/pickproduct_idsql_dialog.ui \
    sources/pickpart_dialog.ui \
    sources/pickname_dialog.ui \
    sources/pickfilter_dialog.ui \
    sources/pickcoord_dialog.ui \
    sources/pickcombo_dialog.ui \
    sources/pickbin_single_dialog.ui \
    sources/pickbin_dialog.ui \
    sources/pick_product_id_dialog.ui \
    sources/pick_parameterfilter_dialog.ui \
    sources/pick_export_wafermap_dialog.ui \
    sources/pick_audit_filter_dialog.ui \
    sources/onefile_fileslist_dialog.ui \
    sources/mixfiles_fileslist_dialog.ui \
    sources/mergefiles_fileslist_dialog.ui \
    sources/gex_debug_dialog.ui \
    sources/getstring_dialog.ui \
    sources/filter_dialog.ui \
    sources/edit_bin_entry_dialog.ui \
    sources/drill_guardbanding_dialog.ui \
    sources/drill_custom_parameter_dialog.ui \
    sources/conversionprogress_dialog.ui \
    sources/comparefiles_fileslist_dialog.ui \
    sources/calendar_dialog.ui \
    sources/bincolors_dialog.ui \
    sources/auto_repair_dialog.ui \
    sources/pick_fields_dialog.ui\
    sources/filter_log_dialog.ui \
    sources/plugin_eventdump_dialog.ui \
    sources/pat_report_ft_gui.ui \
    sources/common_widgets/multi_select_input_dialog.ui

macx-clang{
    SOURCES += \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/adler32.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/compress.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/crc32.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/deflate.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/gzclose.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/gzlib.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/gzread.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/gzwrite.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/infback.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/inffast.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/inflate.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/inftrees.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/trees.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/uncompr.c \
        $(QTSRCDIR)/qtbase/src/3rdparty/zlib/zutil.c

    QMAKE_CFLAGS_WARN_ON += -Wno-unknown-warning-option -Wno-shift-negative-value
}

DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/wafer \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/pat \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/xml \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/gtm \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-oc/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-pb/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-pb/include \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/detachable_tab_window_container

RC_FILE = sources/gex.rc

equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,2){
    UI_DIR = $(DEVDIR)/galaxy_products/gex_product/gex/sources/ui
}


! linux-g++-64:message(Qt source dir : $(QTSRCDIR))

macx-clang {
    INCLUDEPATH += /System/Library/Frameworks/OpenGL.framework/Headers
}

INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/zlib

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/include \
    $(DEVDIR)/other_libraries/chartdirector/include \
    $(DEVDIR)/other_libraries/libqglviewer/include \
    $(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/src \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/wafer \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/pat \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/gtm \
    $(DEVDIR)/galaxy_products/gex_product/services \
    $(DEVDIR)/galaxy_products/gex_product/gex-oc/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-pb/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtc \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/detachable_tab_window_container \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/on_the_fly_report \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/monitoring \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/monitoring/sya \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/monitoring/spm \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/common \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/external_services/statistical_agents \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources/customizedWidgets \
    $(QTSRCDIR)

#    $(DEVDIR)/galaxy_products/gex_product/gex-log/include

equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,2){
   INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources/ui
}
# needed to include gtl_core.h in gtm sources
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include
DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include

# TO FIX mocinclude.tmp error (see QTBUG-23196 && QTBUG-6733)
mocinclude.CONFIG += fix_target

# needed for odp export
INCLUDEPATH += $(DEVDIR)/other_libraries/quazip-0.4.4

PRE_TARGETDEPS += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/lib$$gexTargetName(gtl_core).a

# as galaxy_common.pri already preset GALAXY_LIBS, do NOT reset it with =
GALAXY_LIBS += -l$$gexTargetName(gqtl_stdf) \
    -l$$gexTargetName(gqtl_filelock) \
     -l$$gexTargetName(gqtl_ftp) \
    -l$$gexTargetName(gqtl_datakeys) \
#    -l$$gexTargetName(gqtl_ziparchive) \
    -l$$gexTargetName(gqtl_service) \
    -l$$gexTargetName(gqtl_ws) \
    -l$$gexTargetName(gexpb) \
    -l$$gexTargetName(gqtl_statsengine) \
    -l$$gexTargetName(gqtl_patcore) \
    -l$$gexTargetName(gqtl_parser) \
    -l$$gexTargetName(gqtl) \
    -l$$gexTargetName(gstdl)\
    -lgs_data\
    -lgs_gtl_traceability

# Alex/Seb: win32 now also needs to link with lz
unix:GALAXY_LIBS += -lz
# some say there is no libz.a on some windows build machine... Let s add it in debug only:
CONFIG(debug, debug|release) {
  # libz should be here : c:\Qt\Qt5.1.1\Tools\mingw48_32\i686-w64-mingw32\lib\libz.a
  GALAXY_LIBS += -lz
}

win32: GALAXY_LIBS += -lws2_32 -lgdi32

GALAXY_LIBSPATH +=\
 -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/gex-pb/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/bin\

OTHERS_LIBSPATH = \
    -L$(DEVDIR)/other_libraries/libqglviewer/lib/$$OSFOLDER \
    -L$(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/$$OSFOLDER

# R
win32: OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER
linux-g++-32 {
    OTHERS_LIBSPATH += -L/usr/lib/R/lib -lR -lRblas
    OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas
}
linux-g++-64 {
    OTHERS_LIBSPATH += -L/usr/lib64/R/lib -lR -lRblas
    OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas
}
macx-clang {
    OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/R/lib/mac -lR -lRblas
}

# tufao
OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/tufao/0.8/lib/$$OSFOLDER\
 -l$$gexTargetName(tufao)

CONFIG(debug, debug|release) {
  # quazip needed for creating multiple files zip archive
  OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/quazip-0.4.4/lib/$$OSFOLDER -l$$gexTargetName(quazip)
}

# qglviewer : probably unusefull for daemon build
win32: LIBS += -l$$gexTargetName(qglviewer)2
unix:  LIBS += -l$$gexTargetName(qglviewer)
macx:  LIBS += -l$$gexTargetName(qglviewer)

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

contains(QMAKE_COMPILER_DEFINES, _WIN64) {
    OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/chartdirector/lib/win64
} else {
    OTHERS_LIBSPATH += -L$(DEVDIR)/other_libraries/chartdirector/lib/$$OSFOLDER
}

LIBS += $$GALAXY_LIBS

win*-msvc*: LIBS += -lshell32 -lAdvAPI32 -lopengl32 -lglu32 -luser32
win*-msvc*: QMAKE_LFLAGS += /VERBOSE:LIB
LIBS += $$GALAXY_LIBSPATH \
    $$OTHERS_LIBSPATH

# ChartDirector
#win32: LIBS += -lchartdir50
win32: LIBS += -lchartdir51
unix:  LIBS += -lchartdir

#zlib
unix: LIBS+= -lz

gex_no_webkit {
    QT -= webkit
    DEFINES += GEX_NO_WEBKIT
}
gex_no_js_solaris {
    DEFINES += GEX_NO_JS_SOLARIS
}

# Strangely on Ubuntu 11 Qt 4.8.1  there is no linux-g++-* defined but at least a 'linux-g++'
linux-g++*: LIBS += -lGLU
linux-g++-*: LIBS += -ldl   # needed for some sqlite3.o versions

! linux-g++-64:message(LIBS : $$LIBS)

! linux-g++-64:message(QT : $$QT)

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++*:DEFINES += _FILE_OFFSET_BITS=64

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/license-provider-plugin/base-lp/sources
DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/plugins/license-provider-plugin/base-lp/sources
LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER -l$$gexTargetName(base_lp)

LIBS += -l$$gexTargetName(qx_std_utils)
LIBS += -l$$gexTargetName(qx_bin_mapping)
