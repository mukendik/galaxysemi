# -------------------------------------------------
# Project created by QtCreator 2009-09-22T15:55:52
# -------------------------------------------------
QT += network \
    qt3support
TARGET = vfei_simulator
TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

SOURCES += main.cpp \
    licensestatus.cpp \
    EventLM.cpp \
    ConnectionLM.cpp
HEADERS += vfei_simulator.h \
    resource.h \
    licensestatus.h \
    EventLM.h \
    ConnectionLM.h
OTHER_FILES += vfei_simulator.rc \
    gex_ls.ico
FORMS += licensestatus_dialogbase.ui
RESOURCES += vfei_simulator.qrc
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

GALAXY_LIBS = -l$$gexTargetName(gstdl_utils) \
	-l$$gexTargetName(gstdl_errormgr) \
	-l$$gexTargetName(gstdl_info) \
	-l$$gexTargetName(gstdl_utils_c) \
	-l$$gexTargetName(gqtl_sysutils)

GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \
	-L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH
