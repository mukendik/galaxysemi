TEMPLATE = subdirs

SUBDIRS = \
# other libs
other_libraries/libqglviewer/qglviewer \
other_libraries/qtpropertybrowser-2.5_1-commercial/qtpropertybrowser.pro \
other_libraries/tufao/0.8/libtufao.pro \
other_libraries/quazip-0.4.4/quazip/quazip.pro \
# std libs
galaxy_libraries/galaxy_std_libraries/qx_std_utils\
galaxy_libraries/galaxy_std_libraries/gstdl.pro \
galaxy_libraries/galaxy_std_libraries/gs_data/gs_data.pro \
galaxy_libraries/galaxy_std_libraries/gs_data/gs_data_sqlite.pro \
galaxy_libraries/galaxy_std_libraries/gs_gtl_traceability/gs_gtl_traceability.pro \
galaxy_libraries/galaxy_std_libraries/external_services/external_services.pro \
# qt libs
galaxy_libraries/galaxy_qt_libraries/gqtl.pro \
galaxy_libraries/galaxy_qt_libraries/gqtl_stdf \
galaxy_libraries/galaxy_qt_libraries/gqtl_parser \
galaxy_libraries/galaxy_std_libraries/qx_bin_mapping \
galaxy_libraries/galaxy_qt_libraries/gqtl_datakeys \
galaxy_libraries/galaxy_qt_libraries/gqtl_filelock \
galaxy_libraries/galaxy_qt_libraries/gqtl_ftp \
galaxy_libraries/galaxy_qt_libraries/gqtl_log \
galaxy_libraries/galaxy_qt_libraries/gqtl_service \
galaxy_libraries/galaxy_qt_libraries/gqtl_sqlbrowser \
galaxy_libraries/galaxy_qt_libraries/gqtl_svg \
galaxy_libraries/galaxy_qt_libraries/gqtl_webserver \
galaxy_libraries/galaxy_qt_libraries/gqtl_statsengine \
galaxy_libraries/galaxy_qt_libraries/gqtl_patcore \
# galaxy products
galaxy_products/gex_product/plugins/gexdb_plugin_base \
galaxy_products/gex_product/plugins/directory-access-plugin \
galaxy_products/gex_product/plugins/license-provider-plugin \
galaxy_products/gex_product/gex-tester/gtl/gtl-core/gtl_core.pro \
galaxy_products/gex_product/gex-pb/libgexpb.pro \
galaxy_products/gex_product/gex-oc/libgexoc.pro \
galaxy_products/gex_product/plugins/gexdb_plugin_galaxy \
galaxy_products/gex_product/plugins/gexdb_plugin_medtronic
