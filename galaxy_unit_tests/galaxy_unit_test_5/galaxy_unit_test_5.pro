TEMPLATE = app
CONFIG += console
CONFIG += qt
QT += xml
QT -= widgets
QT -= network
QT -= script
QT -= sql
QT -= gui
QT -= opengl

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_parser/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/common/promis_interpreter \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/hvm_ws \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ft \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_bin_mapping/sources/lvm_ws

SOURCES += main.cpp

# HEADERS += stdfrecord.h


LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += $$GALAXY_LIBS
LIBS += $$GALAXY_LIBSPATH
LIBS += -l$$gexTargetName(gqtl)
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl_stdf)
LIBS += -l$$gexTargetName(gexdb_plugin_base)
LIBS += -l$$gexTargetName(gqtl_parser)
LIBS += -l$$gexTargetName(qx_std_utils)
LIBS += -l$$gexTargetName(qx_bin_mapping)
linux-g++-64: LIBS += -ldl

win32:LIBS += libwsock32 libws2_32 libole32 liboleaut32 libsnmpapi

win32-g++: QMAKE_CXXFLAGS += -posix
