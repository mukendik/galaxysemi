#-------------------------------------------------
#
# Project created by QtCreator 2018-07-11T10:13:29
#
#-------------------------------------------------

QT = core

TEMPLATE = lib

include($(DEVDIR)/galaxy_common.pri)

DEFINES += QX_BIN_MAPPING_API

# look into general defines to rule how the library is built
contains(DEFINES, QX_SHARED_API) : {
    CONFIG += shared
} else : {
    CONFIG += static
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    sources/lvm_ws/lvm_ws_bin_map_store.cpp \
    sources/common/bin_map_store_base.cpp \
    sources/hvm_ft/hvm_ft_bin_map_store.cpp \
    sources/lvm_ft/lvm_ft_final_test_bin_map_store.cpp \
    sources/hvm_ws/hvm_ws_bin_map_store.cpp \
    sources/lvm_ft/lvm_ft_sort_entries_bin_map_store.cpp \
    sources/common/bin_map_item_base.cpp \
    sources/common/bin_mapping_exceptions.cpp \
    sources/common/queryable_by_bin_name_store.cpp \
    sources/common/queryable_by_test_name_store.cpp \
    sources/common/queryable_by_test_number_store.cpp \
    sources/hvm_ws/hvm_ws_bin_map_store_specktra.cpp \
    sources/hvm_ws/hvm_ws_bin_map_store_fet_test.cpp \
    sources/common/promis_interpreter/promis_item_base.cpp \
    sources/common/promis_interpreter/promis_exceptions.cpp \
    sources/promis_interpreter/lvm_ft_subcon_data/lvm_ft_subcon_data_promis_interpreter.cpp \
    sources/common/promis_interpreter/promis_interpreter_base.cpp \
    sources/common/tabular_file_line_fields.cpp \
    sources/promis_interpreter/lvm_ft_lotlist/lvm_ft_lotlist_promis_interpreter.cpp \
    sources/promis_interpreter/lvm_wt/lvm_wt_promis_interpreter.cpp \
    sources/promis_interpreter/hvm_wt/hvm_wt_promis_interpreter.cpp \
    sources/promis_interpreter/lvm_ft/lvm_ft_promis_intepreter.cpp \
    sources/promis_interpreter/hvm_ft/hvm_ft_promis_interpreter.cpp \
    sources/lvm_ft/lvm_ft_sort_entries_new_bin_map_store.cpp \
    sources/lvm_ft/lvm_ft_sort_entries_base_bin_map_store.cpp

HEADERS += \
    sources/bin_mapping_api.h \
    sources/common/bin_mapping_exceptions.h \
    sources/common/bin_map_item_base.h \
    sources/common/bin_map_store_factory.h \
    sources/common/bin_map_store_base.h \
    sources/hvm_ft/hvm_ft_bin_map_item.h \
    sources/hvm_ft/hvm_ft_bin_map_store.h \
    sources/lvm_ft/lvm_ft_bin_map_item.h \
    sources/lvm_ft/lvm_ft_final_test_bin_map_store.h \
    sources/lvm_ws/lvm_ws_bin_map_store.h \
    sources/lvm_ws/lvm_ws_bin_map_item.h \
    sources/hvm_ft/hvm_ft_bin_map_item.h \
    sources/hvm_ft/hvm_ft_bin_map_store.h \
    sources/hvm_ws/hvm_ws_bin_map_item.h \
    sources/hvm_ws/hvm_ws_bin_map_store.h \
    sources/lvm_ft/lvm_ft_sort_entries_bin_map_store.h \
    sources/common/bin_map_store_predicates.h \
    sources/common/queryable_by_bin_name_store.h \
    sources/common/queryable_by_test_name_store.h \
    sources/common/queryable_by_test_number_store.h \
    sources/hvm_ws/hvm_ws_bin_map_store_specktra.h \
    sources/hvm_ws/hvm_ws_bin_map_store_fet_test.h \
    sources/common/validity_check_for_bin.h \
    sources/common/promis_interpreter/promis_item_base.h \
    sources/promis_interpreter/lvm_ft_subcon_data/lvm_ft_subcon_data_promis_item.h \
    sources/common/promis_interpreter/promis_exceptions.h \
    sources/promis_interpreter/lvm_ft_subcon_data/lvm_ft_subcon_data_promis_interpreter.h \
    sources/common/promis_interpreter/promis_interpreter_base.h \
    sources/common/promis_interpreter/promis_interpreter_factory.h \
    sources/common/tabular_file_line_fields.h \
    sources/promis_interpreter/lvm_ft_lotlist/lvm_ft_lotlist_promis_interpreter.h \
    sources/promis_interpreter/lvm_ft_lotlist/lvm_ft_lotlist_promis_item.h \
    sources/promis_interpreter/lvm_wt/lvm_wt_promis_interpreter.h \
    sources/promis_interpreter/lvm_wt/lvm_wt_promis_item.h \
    sources/promis_interpreter/hvm_wt/hvm_wt_promis_interpreter.h \
    sources/promis_interpreter/hvm_wt/hvm_wt_promis_item.h \
    sources/promis_interpreter/lvm_ft/lvm_ft_promis_intepreter.h \
    sources/promis_interpreter/lvm_ft/lvm_ft_promis_item.h \
    sources/promis_interpreter/hvm_ft/hvm_ft_promis_interpreter.h \
    sources/promis_interpreter/hvm_ft/hvm_ft_promis_item.h \
    sources/lvm_ft/lvm_ft_sort_entries_new_bin_map_store.h \
    sources/lvm_ft/lvm_ft_sort_entries_base_bin_map_store.h

INCLUDEPATH += \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_subcon_data \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_lotlist \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common/promis_interpreter \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(qx_std_utils)

RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(qx_bin_mapping)
