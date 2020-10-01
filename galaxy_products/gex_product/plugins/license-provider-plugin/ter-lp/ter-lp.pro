#-------------------------------------------------
#
# Project created by QtCreator 2015-08-28T15:49:02
#
#-------------------------------------------------

win32:{

DEFINES += MODULE=GEXLP

include($(DEVDIR)/galaxy_common.pri)

QT += gui
QT += network xml
QT += widgets
QT += axcontainer

DEFINES += LICENSE_PROVIDER_LIBRARY

TEMPLATE = lib
CONFIG += dll debug_and_release

DESTDIR = $$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
TARGET = $$gexTargetName(ter_lp)

LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER -l$$gexTargetName(base_lp) \
        -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl) \
        -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)

win32:LIBS +=  -lsnmpapi -lPsapi -lws2_32

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
                $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include



SOURCES += \
    ter_license_provider.cpp \
    ter-lp.cpp

HEADERS  += \
    ter_license_provider.h


FORMS    += \
    termainwindow.ui

CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

}

