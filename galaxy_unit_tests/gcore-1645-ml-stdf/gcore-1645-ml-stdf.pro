#-------------------------------------------------
#
# Project created by QtCreator 2014-11-04T10:18:24
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET   = gcore-1645-ml-stdf
TEMPLATE = app
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

SOURCES += main.cpp

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

LIBS += -l$$gexTargetName(gqtl)
LIBS += -l$$gexTargetName(gstdl)
LIBS += -l$$gexTargetName(gqtl_stdf)

