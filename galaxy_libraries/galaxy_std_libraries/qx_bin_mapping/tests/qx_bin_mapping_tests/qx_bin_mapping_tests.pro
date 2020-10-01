QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

HEADERS += \
    test_overall_bin_mapping.h \
    test_doubles/testable_bin_map_store.h

SOURCES += \
    test_overall_bin_mapping.cpp \
    test_doubles/testable_bin_map_store.cpp \
    details/test_overall_bin_mapping.cpp

INCLUDEPATH += \
    $$PWD/test_doubles \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/ \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft

LIBS += \
    -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER \
    -l$$gexTargetName(qx_std_utils) \
    -l$$gexTargetName(qx_bin_mapping)

DEFINES += QX_BIN_MAPPING_TESTS_PROJECT_DIRECTORY=\\\"$$PWD\\\"

MOC_DIR = $$OBJECTS_DIR
