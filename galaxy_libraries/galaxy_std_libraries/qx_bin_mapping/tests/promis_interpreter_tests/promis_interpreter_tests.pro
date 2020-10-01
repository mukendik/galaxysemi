QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

SOURCES += \
    test_overall_promis_interpreter.cpp \
    details/test_overall_promis_interpreter.cpp \
    test_doubles/testable_promis_interpreter.cpp

HEADERS += \
    test_overall_promis_interpreter.h \
    test_doubles/testable_promis_interpreter.h

INCLUDEPATH += \
    $$PWD/test_doubles \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common/ \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common/promis_interpreter \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_subcon_data \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft_lotlist \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_wt \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/promis_interpreter/hvm_ft

DEFINES += PROMIS_INTERPRETER_TESTS_PROJECT_DIRECTORY=\\\"$$PWD\\\"

LIBS += \
    -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER \
    -l$$gexTargetName(qx_std_utils) \
    -l$$gexTargetName(qx_bin_mapping)

MOC_DIR = $$OBJECTS_DIR
