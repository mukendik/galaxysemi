#-------------------------------------------------
#
# Project created by QtCreator 2012-06-06T10:30:50
#
#-------------------------------------------------

DEFINES += MODULE=GQTL_SVG_UNITTEST

QT += core gui xml opengl
QT += svg

TARGET = gqtl_svg_unittest
TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

SOURCES += main.cpp \
        mainwindow.cpp \
        $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/sources/libgexsvg.cpp

HEADERS  += mainwindow.h

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

LIBS +=  -l$$gexTargetName(gstdl)
LIBS +=  -l$$gexTargetName(gqtl)

win32: LIBS += -lPsapi

FORMS    += mainwindow.ui
