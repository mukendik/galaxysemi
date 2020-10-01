TEMPLATE = app
CONFIG += console
CONFIG += qt
QT -= widgets
QT -= xml
QT -= network
QT -= script
QT -= sql
QT -= gui
QT -= opengl

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

SOURCES += main.cpp

HEADERS +=

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += $$GALAXY_LIBS
LIBS += $$GALAXY_LIBSPATH
LIBS += -l$$gexTargetName(gqtl)
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gtl_core)
LIBS += -l$$gexTargetName(gqtl_stdf)
LIBS += -l$$gexTargetName(gqtl_datakeys)

LIBS += -L$(DEVDIR)/galaxy_products/gex_product/bin -lgs_data
