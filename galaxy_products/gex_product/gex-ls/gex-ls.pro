# -------------------------------------------------
# Project created by QtCreator 2009-03-25T11:56:44
# -------------------------------------------------
include($(DEVDIR)/galaxy_common.pri)
GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)

QT += network
QT += widgets
TEMPLATE = app
CONFIG += debug_and_release

#GALAXY_LIBS = -l$$gexTargetName(gqtl)

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
TARGET = $$gexTargetName(gex-ls)

SOURCES += sources/main.cpp \
    sources/licensestatus.cpp \
    sources/cryptofile.cpp \
    sources/statisticmanager.cpp \
    sources/htmlreport.cpp \
    sources/historyeventmanager.cpp \
    sources/eventlm.cpp \
    sources/connectionlm.cpp
HEADERS += sources/resource.h \
    sources/licensestatus.h \
    sources/gex-ls.h \
    sources/cryptofile.h \
    sources/statisticmanager.h \
    sources/htmlreport.h \
    sources/historyeventmanager.h \
    sources/eventlm.h \
    sources/connectionlm.h
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
				$(DEVDIR)/galaxy_products/gex_product/include \
				$(DEVDIR)/galaxy_products/gex_product/gex-ls/sources/ui
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
	$(DEVDIR)/galaxy_products/gex_product/include \
	$$PWD/sources
FORMS += sources/licensestatus_dialogbase.ui
UI_DIR += $(DEVDIR)/galaxy_products/gex_product/gex-ls/sources/ui
RESOURCES += sources/gex_ls.qrc
RC_FILE = sources/gex_ls.rc

CONFIG(debug, debug|release) {
	OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
	OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH

#PLEASE DON'T REMOVE THIS LINE
#user_custom.pri must be used to customize the .pro in your environment.
#Use this in order to do tests which won't be committed on the SCM.
exists(user_custom.pri) {
	include(user_custom.pri)
}

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

# To be able to use GetProcessMemoryInfo() on Windows 7 and earlier (Win XP)
win32:DEFINES += PSAPI_VERSION=1
