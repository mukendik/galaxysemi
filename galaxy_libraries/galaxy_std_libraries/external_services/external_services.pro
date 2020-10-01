include($(DEVDIR)/galaxy_common.pri)
include($(DEVDIR)/galaxy_warnings.pri)

TARGET = external_services
TEMPLATE = lib
CONFIG += staticlib
CONFIG += debug_and_release

QT = core network

DESTDIR    = ../lib/$$OSFOLDER
DLLDESTDIR = ../lib/$$OSFOLDER

INCLUDEPATH += \
    sources/common \
    sources/statistical_agents \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/ \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/qx_std_utils/sources/string_utils

HEADERS += \
    sources/common/DumpableAs.h \
    sources/common/GlobalNetworkAccessManager.h \
    sources/common/HttpChannel.h \
    sources/common/NullableOf.h \
    sources/statistical_agents/Exceptions.h \
    sources/statistical_agents/JobDefinitions.h

SOURCES += \
    sources/common/GlobalNetworkAccessManager.cpp \
    sources/common/HttpChannel.cpp \
    sources/statistical_agents/JobDefinitions.cpp

win*-msvc* {
  QMAKE_CXXFLAGS_WARN_ON += /wd4267
}

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(qx_std_utils)
