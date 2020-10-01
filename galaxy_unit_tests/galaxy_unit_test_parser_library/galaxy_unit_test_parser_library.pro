CONFIG += console
CONFIG += testcase
CONFIG += insignificant_test
CONFIG -= app_bundle
QT += testlib
QT -= widgets
QT += xml
QT -= network
QT -= script
QT -= sql
QT -= gui
QT -= opengl

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common
INCLUDEPATH += $(DEVDIR)/galaxy_unit_tests/galaxy_unit_test_parser_library/include

!macx-clang | win32-g++ {
    INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/zlib

    include($(QTSRCDIR)/qtbase/src/3rdparty/zlib.pri)

    # zlib, that is built on this branch need those flags for c language sources.
    QMAKE_CFLAGS_WARN_ON += -Wno-unknown-warning-option
    QMAKE_CFLAGS_WARN_ON += -Wno-shift-negative-value
}

INCLUDEPATH += \
$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
                $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $$PWD/sources

SOURCES +=\
    sources/TestGqtlParser.cpp \
    sources/parser_test_expected_result.cpp
HEADERS +=\
    include/TestGqtlParser.h \
    include/parser_test_expected_result.h

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += $$GALAXY_LIBS
LIBS += $$GALAXY_LIBSPATH
LIBS += -l$$gexTargetName(gqtl_parser)
LIBS += -l$$gexTargetName(gqtl_stdf)
LIBS += -l$$gexTargetName(gqtl)
LIBS += -l$$gexTargetName(gstdl)

LIBS += -l$$gexTargetName(qx_std_utils)
LIBS += -l$$gexTargetName(qx_bin_mapping)

!win* : LIBS += -lz

linux-g++-*: LIBS += -ldl
