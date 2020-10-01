SOURCES += sources/tb_merge_retest_dialog.cpp \
    sources/tb_import_csv_wizard_dialog.cpp \
    sources/tb_admin.cpp \
    sources/tb_csv_operations_dialog.cpp \
    sources/report_page_file_audit.cpp \
    sources/regexp_validator.cpp \
    sources/db_key_data.cpp \
    sources/db_key_static_row_items.cpp \
    sources/db_key_dyn_row_items.cpp\
    sources/db_keys_editor.cpp \
    $$PWD/sources/tb_merge_retest.cpp
HEADERS += sources/tb_toolbox.h \
    sources/tb_merge_retest_dialog.h \
    sources/tb_import_csv_wizard_dialog.h \
    sources/tb_csv_operations_dialog.h \
    sources/report_page_file_audit.h \
    sources/regexp_validator.h \
    sources/db_keys_editor.h \
    sources/db_key_data.h \
    sources/db_key_static_row_items.h \
    sources/db_key_dyn_row_items.h \
    sources/tb_merge_retest.h
FORMS += sources/tb_merge_retest_dialog.ui \
    sources/tb_import_csv_wizard_dialog.ui \
    sources/tb_convert_dialog.ui \
    sources/tb_csv_operations_dialog.ui \
    sources/db_keys_editor.ui
