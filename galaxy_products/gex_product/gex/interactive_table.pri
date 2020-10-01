SOURCES += sources/drill_table.cpp \
    sources/drill_device_table.cpp \
    sources/drill_parametric_table.cpp \
    sources/drill_bin_table.cpp \
    sources/gex_wizard_table.cpp \
    sources/drill_func_table.cpp \
    $$PWD/sources/loading.cpp
HEADERS += sources/drill_table.h \
    sources/drill_parametric_table.h \
    sources/drill_device_table.h \
    $$PWD/sources/loading.h
FORMS += sources/drilltable_dialog.ui \
         sources/interactive_table_find_dialog.ui
