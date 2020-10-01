# -------------------------------------------------
# Project created by QtCreator 2009-03-25T11:29:20
# -------------------------------------------------
QT += network
QT += widgets
TEMPLATE = app
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
TARGET = $$gexTargetName(gex-lm)

SOURCES += sources/read_system_info.cpp \
    sources/main.cpp \
    sources/hostid_snmpapi.cpp \
    sources/hostid_mibaccess.cpp \
    sources/gexlm_server.cpp \
    sources/gexlm_mainobject.cpp \
    sources/cryptofile.cpp \
	sources/activation_serverkey.cpp
HEADERS += sources/read_system_info.h \
    sources/iptypes.h \
    sources/hostid_mibaccess.h \
    sources/gexlm_service.h \
    sources/gexlm_server.h \
    sources/gexlm_mainobject.h \
    sources/gexlm.h \
    sources/cryptofile.h \
    sources/activation_serverkey.h \
	sources/resource.h
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
	$(DEVDIR)/galaxy_products/gex_product/gex-lm/sources/ui
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
	$(DEVDIR)/galaxy_products/gex_product/include \
	$$PWD/sources
FORMS += sources/gexlm_config_dialogbase.ui \
    sources/activation_serverkey_dialogbase.ui
#UI_DIR += $(DEVDIR)/galaxy_products/gex_product/gex-lm/sources/ui
RESOURCES += sources/gex_lm.qrc
RC_FILE = sources/gex_lm.rc

# now set by glaxy_common
#win32:LIBS += -lwsock32  -lsnmpapi

# gqtl is now already included in galaxy_common
#GALAXY_LIBS += -l$$gexTargetName(gqtl)
GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)
GALAXY_LIBS += -l$$gexTargetName(gqtl_service)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

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
