QT += core
QT += gui
QT += widgets

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

SOURCES += main.cpp \
    sources/gexdatabasekeyscontent_ut.cpp

HEADERS += \
    sources/gexdatabasekeyscontent_ut.h

INCLUDEPATH += \
    $(DEVDIR)/galaxy_products/gex_product/gex/sources \
    $(DEVDIR)/galaxy_products/gex_product/plugins/include \
    $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

LIBS += $$GALAXY_LIBSPATH
LIBS += $$GALAXY_LIBS
LIBS += -L$$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gqtl_datakeys)

LIBS += -L$(DEVDIR)/galaxy_products/gex_product/bin -lgs_data
