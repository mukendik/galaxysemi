DEFINES += MODULE=GEXLP

include($(DEVDIR)/galaxy_common.pri)

QT += gui xml network
QT += widgets

DEFINES += LICENSE_PROVIDER_LIBRARY

TEMPLATE = lib
CONFIG += dll debug_and_release

DESTDIR = $$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
TARGET = $$gexTargetName(fnp_lp)

LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER -l$$gexTargetName(base_lp)\
        -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)

#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin/plugins/lp
DLLDESTDIR	= $(PWD)/../../../../bin/plugins/lp

unix {
    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/plugins/lp
    macx-clang {
        QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET*.dylib $(DEVDIR)/galaxy_products/gex_product/bin/plugins/lp
    }
}

INCLUDEPATH += ../base-lp/sources ../base-lp \
                $(DEVDIR)/galaxy_products/gex_product/include \
                $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
                $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/gstdl_fnp_proxy/sources \
                $(DEVDIR)/other_libraries/fnp-toolkit/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

SOURCES += sources/fnp_license_provider.cpp \
           sources/fnp-lp.cpp \
    sources/download_license.cpp

HEADERS += sources/fnp_license_provider.h \
    sources/download_license.h \
../base-lp/sources/server_file_descriptorIO_factory.h



CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

#DLLDESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin/lp_plugins
#unix {
#    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/lp_plugins
#}

#win32 {
#    QMAKE_POST_LINK += $(COPY_FILE) $(DESTDIR)/fnp_provider*.dll $(DEVDIR)/galaxy_products/gex_product/bin/lp_plugins
#    QMAKE_POST_LINK += $(COPY_FILE) $(DESTDIR)"FNP_Act_Installer.dll" $(DEVDIR)/galaxy_products/gex_product/bin/lp_plugins
#}
