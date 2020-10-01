QT += testlib gui xml

include($(DEVDIR)/galaxy_products/pat_prod_gtrigger/testable_g_trigger.pri)

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    g_trigger_tests.cpp \
    details/g_trigger_tests.cpp \
    details/simple_stdf_to_atdf_converter.cpp

HEADERS += \
    g_trigger_tests.h \
    test_doubles/testable_g_trigger_engine.h \
    details/testable_g_trigger_engine_worker.h \
    details/simple_stdf_to_atdf_converter.h

DESTDIR = $$PWD/data/output
TARGET = g-trigger-tests
DEFINES += TEST_PROGRAM_DIRECTORY=\\\"$$DESTDIR\\\"
DEFINES += TEST_PROGRAM_NAME=\\\"$$TARGET\\\"

INCLUDEPATH += \
    $(DEVDIR)/galaxy_products/pat_prod_gtrigger/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/ \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/ \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/qx_concurrent/include

LIBS += -L$$G_TRIGGER_OBJECTS_DIR
LIBS += \
    -lg-trigger_engine.o \
    -lprofile.o \
    -lg-trigger.o \
    -lmoc_g-trigger_engine.o \
    -lg-trigger_engine_eagle.o \
    -lg-trigger_engine_fet_test.o \
    -lg-trigger_engine_spektra.o \
    -lg-trigger_engine_create_trigger_files.o \
    -lfettest_nopat.o \
    -lcstdf.o

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += \
    -l$$gexTargetName(gqtl_parser) \
    -l$$gexTargetName(gqtl_patcore) \
    -l$$gexTargetName(gqtl_stdf) \
    -l$$gexTargetName(qx_concurrent)

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += \
    -l$$gexTargetName(gstdl) \
    -l$$gexTargetName(qx_std_utils) \
    -l$$gexTargetName(qx_bin_mapping)