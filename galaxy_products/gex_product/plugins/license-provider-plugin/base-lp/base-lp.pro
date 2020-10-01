DEFINES += MODULE=GEXLP

include($(DEVDIR)/galaxy_common.pri)

DEFINES += LICENSE_PROVIDER_LIBRARY

QT += gui
QT += network xml
QT += widgets

TEMPLATE = lib
CONFIG += shared debug_and_release

DESTDIR = $$PWD/../../lib/$$OSFOLDER
#DESTDIR = $$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
TARGET = $$gexTargetName(base_lp)

#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin/
#DLLDESTDIR	= $(PWD)/../../../../bin/
#unix {
#    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/
#}

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl) \
        -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)
win32:LIBS +=  -lsnmpapi -lPsapi -lws2_32

INCLUDEPATH +=  $(DEVDIR)/galaxy_products/gex_product/include \
                $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

win32-g++ : INCLUDEPATH += $(QTDIR)/include

FORMS += \
    sources/license_provider_dialog.ui \
    sources/cpuchoices.ui \
    sources/evaluation_dialog.ui

UI_HEADERS_DIR += sources
UI_SOURCES_DIR += sources

SOURCES += sources/license_provider.cpp \
           sources/license_provider_common.cpp \
           sources/product_info.cpp \
           sources/license_provider_manager.cpp \
           sources/license_provider_thread.cpp \
           sources/license_provider_dialog.cpp \
           sources/license_provider_profile.cpp \
           sources/read_system_info.cpp \
           sources/hostid_mibaccess.cpp \
    sources/cpuchoices.cpp \
    sources/evaluation_dialog.cpp \
    sources/server_file_descriptorIO_factory.cpp \
    sources/xml_server_file_descriptorIO.cpp \
    sources/server_file_descriptorIO.cpp \
    sources/json_server_file_descriptorIO.cpp

HEADERS += sources/license_provider_global.h \
           sources/license_provider_common.h \
           sources/license_provider.h \
           sources/product_info.h \
           sources/license_provider_manager.h \
           sources/license_provider_thread.h \
           sources/license_provider_dialog.h \
           sources/license_provider_profile.h \
           sources/read_system_info.h \
           sources/hostid_mibaccess.h \
    sources/cpuchoices.h \
    sources/evaluation_dialog.h \
    sources/server_file_descriptorIO_factory.h \
    sources/server_file_descriptorIO.h \
    sources/xml_server_file_descriptorIO.h \
    sources/json_server_file_descriptorIO.h

CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

CONFIG(debug, debug|release) {
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gstdl)
}
