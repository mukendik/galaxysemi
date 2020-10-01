# todo : remove GUI files in daemon build
#!contains(DEFINES, GSDAEMON):
SOURCES += sources/tb_pat_recipe_wizard_dialog.cpp \
    sources/pat/pat_process_ws.cpp \
    sources/pat/pat_gts_station.cpp \
    sources/pat/pat_widget_ft.cpp \
    sources/pat/pat_report_ws.cpp \
    sources/pat/pat_report_ft.cpp \
    sources/pat/pat_db_traceability_v2.cpp \
    sources/pat/pat_recipe_editor.cpp \
    sources/pat/pat_limits_dialog.cpp \
    sources/pat/mv_pat_test_edition.cpp \
    sources/pat/pat_db_traceability_abstract.cpp \
    sources/pat/pat_outlier_finder.cpp \
    sources/pat/pat_outlier_finder_ws_private.cpp \
    sources/pat/pat_outlier_finder_ws.cpp \
    sources/pat/pat_outlier_finder_private.cpp \
    sources/pat/pat_outlier_finder_ft_private.cpp \
    sources/pat/pat_outlier_finder_ft.cpp \
    sources/pat/pat_engine.cpp \
    sources/pat/pat_engine_private.cpp \
    sources/tb_pat_outlier_removal.cpp \
    sources/patman_lib.cpp \
    sources/pat_potatoclusterfinder.cpp \
    sources/pat_plugins.cpp \
#    sources/tcpip_vfei.cpp \
  ../../pat_prod_admin/prod_recipe_file.cpp \
    sources/pat_wafmap_create.cpp \
    sources/pat_shape_analysis.cpp \
    sources/pat_gdbn_engine.cpp \
    sources/pat_gdbn_abstract_algorithm.cpp \
    sources/pat_gdbn_weighting_algo.cpp \
    sources/pat_gdbn_generic_baddies.cpp \
    sources/pat_gdbn_squeeze_algo.cpp \
    sources/pat_gdbn_weighting_ring.cpp \
    sources/pat_gdbn_weighting_matrix_widget.cpp \
    sources/patman_lib_on.cpp \
    sources/pat/pat_outliers.cpp \
    sources/pat_info.cpp \
    sources/pat_context.cpp \
    sources/pat/vfei_server.cpp \
    sources/pat/vfei_config.cpp \
    sources/pat/vfei_client.cpp \
    sources/pat/pat_stdf_updater.cpp \
    sources/pat/pat_sinf_info.cpp \
    sources/pat/pat_external_map_details.cpp \
    sources/pat/pat_db_traceability_v1.cpp \
    sources/pat/pat_recipe_historical_data_gui.cpp \
    sources/pat/pat_nnr_summary.cpp \
    sources/pat/pat_db_traceability_v3.cpp \
    sources/pat/pat_part_filter.cpp \
    sources/pat/reticle_definition_dialog.cpp \
    $$PWD/sources/pat/pat_reticle_engine.cpp \
    $$PWD/sources/pat/pat_reticle_abstract_algorithm.cpp \
    $$PWD/sources/pat/pat_reticle_repeating_pattern.cpp \
    $$PWD/sources/pat/pat_reticle_map_legacy.cpp \
    $$PWD/sources/pat/pat_reticle_map_abstract.cpp \
    $$PWD/sources/pat/pat_reticle_map_extern.cpp \
    $$PWD/sources/pat/pat_reticle_corner_rule.cpp \
    $$PWD/sources/pat/pat_reticle_map_recipe.cpp \
    $$PWD/sources/pat/nnr_definition_dialog.cpp \
    $$PWD/sources/pat/iddq_delta_definition_dialog.cpp \
    $$PWD/sources/pat/cluster_definition_dialog.cpp \
    $$PWD/sources/pat/gdbn_definition_dialog.cpp \
    $$PWD/sources/pat/mask_definition_dialog.cpp \
    $$PWD/sources/pat/pat_reticle_defectivity_check.cpp
HEADERS += sources/tb_pat_recipe_wizard_dialog.h \
    sources/tb_pat_outlier_removal.h \
    sources/patman_lib.h \
    sources/pat_potatoclusterfinder.h \
    sources/pat_plugins.h \
    sources/pat_gdbn_engine.h \
    sources/pat_gdbn_abstract_algorithm.h \
    sources/pat_gdbn_weighting_algo.h \
    sources/pat_gdbn_abstract_baddies.h \
    sources/pat_gdbn_generic_baddies.h \
    sources/pat_gdbn_squeeze_algo.h \
    sources/pat_gdbn_weighting_ring.h \
    sources/pat_gdbn_weighting_matrix_widget.h \
    sources/pat/pat_outliers.h \
    sources/pat_info.h \
    sources/pat_context.h \
    sources/pat/vfei_server.h \
    sources/pat/vfei_config.h \
    sources/pat/vfei_client.h \
    sources/pat/pat_stdf_updater.h \
    sources/pat/pat_sinf_info.h \
    sources/pat/pat_external_map_details.h \
    sources/pat/pat_process_ws.h \
    sources/pat/pat_process_ws_private.h \
    sources/pat/pat_process_ft.h \
    sources/pat/pat_process_ft_private.h \
    sources/pat/pat_gts_station.h \
    sources/pat/pat_widget_ft.h \
    sources/pat/pat_report_ws.h \
    sources/pat/pat_report_ft.h \
    sources/pat/pat_db_traceability_abstract.h \
    sources/pat/pat_db_traceability_v1.h \
    sources/pat/pat_db_traceability_v2.h \
    sources/pat/pat_recipe_editor.h \
    sources/pat/pat_limits_dialog.h \
    sources/pat/mv_pat_test_edition.h \
    sources/pat/pat_outlier_finder.h \
    sources/pat/pat_outlier_finder_ft.h \
    sources/pat/pat_outlier_finder_ft_private.h \
    sources/pat/pat_outlier_finder_private.h \
    sources/pat/pat_outlier_finder_ws.h \
    sources/pat/pat_outlier_finder_ws_private.h \
    sources/pat/pat_engine.h \
    sources/pat/pat_engine_private.h \
    sources/pat/pat_recipe_historical_data_gui.h \
    sources/pat/pat_nnr_summary.h \
    sources/pat/pat_db_traceability_v3.h \
    sources/pat/pat_part_filter.h \
    sources/pat/reticle_definition_dialog.h \
    $$PWD/sources/pat/pat_reticle_engine.h \
    $$PWD/sources/pat/pat_reticle_abstract_algorithm.h \
    $$PWD/sources/pat/pat_reticle_repeating_pattern.h \
    $$PWD/sources/pat/pat_reticle_map_abstract.h \
    $$PWD/sources/pat/pat_reticle_map_legacy.h \
    $$PWD/sources/pat/pat_reticle_map_extern.h \
    $$PWD/sources/pat/pat_reticle_corner_rule.h \
    $$PWD/sources/pat/pat_reticle_map_recipe.h \
    $$PWD/sources/pat/nnr_definition_dialog.h \
    $$PWD/sources/pat/iddq_delta_definition_dialog.h \
    $$PWD/sources/pat/cluster_definition_dialog.h \
    $$PWD/sources/pat/gdbn_definition_dialog.h \
    $$PWD/sources/pat/mask_definition_dialog.h \
    $$PWD/sources/pat/pat_reticle_defectivity_check.h
FORMS += sources/tb_pat_recipe_wizard_dialog.ui \
    sources/tb_pat_limits_dialog.ui \
    sources/tb_pat_dialog.ui \
    sources/gdbn_definition_dialog.ui \
    sources/cluster_definition_dialog.ui \
    sources/nnr_definition_dialog.ui \
    sources/iddq_definition_dialog.ui \
    sources/mask_definition_dialog.ui \
    sources/pat_gdbn_weighting_matrix_widget.ui \
    sources/pat/pat_widget_ft.ui \
    sources/pat/pat_recipe_gui.ui \
    sources/pat/rule_test_edition.ui \
    sources/pat/pat_recipe_historical_data_gui.ui \
    sources/pat/reticle_definition_dialog.ui
