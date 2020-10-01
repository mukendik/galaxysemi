#-------------------------------------------------
#
# Project created by QtCreator 2014-11-04T19:27:37
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += widgets

include($(DEVDIR)/galaxy_common.pri)
LIBS += $$GALAXY_LIBSPATH
LIBS += $$GALAXY_LIBS
#LIBS += -l$$gexTargetName(gqtl)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

TARGET = gqtl_getprocesses
CONFIG   += console
CONFIG   += app_bundle

TEMPLATE = app


SOURCES += main.cpp
