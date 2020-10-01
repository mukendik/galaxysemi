DEFINES += MODULE=GEXDA

TEMPLATE     = lib

CONFIG      += plugin shared

QT += sql xml
QT += widgets

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += ../../include \
              $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

win32-g++ : INCLUDEPATH += $(QTDIR)/include

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

GALAXY_LIBSPATH = -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \

SUBDIRS     += sources

HEADERS     += sources/dir_access_galaxy.h \
              ../../include/dir_access_base.h \
    sources/connector.h \
    sources/sql_connector.h \
    sources/connector_private.h \
    sources/users.h \
    sources/groups.h \
    sources/app_entries.h \
    sources/dir_file.h

SOURCES     += sources/dir_access_galaxy.cpp \
    sources/connector.cpp \
    sources/sql_connector.cpp \
    sources/connector_private.cpp \
    sources/users.cpp \
    sources/groups.cpp \
    sources/app_entries.cpp \
    sources/dir_file.cpp

include(administration_dialog.pri)

DESTDIR = $$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
TARGET = $$gexTargetName(dagalaxy)

DLLDESTDIR	= $(PWD)/../../../../bin/plugins/da
unix {
    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/plugins/da
}

RESOURCES += \
    sources/resources/da_galaxy.qrc


win32 {
  # Fix QRCC error QTBUG-27237
  QMAKE_RCC = $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}rcc.exe
}

CONFIG(debug, debug|release) {
    LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
    LIBS += -l$$gexTargetName(gstdl)
}
