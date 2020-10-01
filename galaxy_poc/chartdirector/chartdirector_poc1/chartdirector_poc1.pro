#-------------------------------------------------
#
# Project created by QtCreator 2013-01-11T19:16:00
#
#-------------------------------------------------

include($(DEVDIR)/galaxy_common.pri)

QT       += core
QT       += gui
QT       += svg
QT       += xml
QT       += xmlpatterns
# usefull for svg ?
QT +=	webkit

TARGET = chartdirector_poc1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$(DEVDIR)/other_libraries/chartdirector/lib/$$OSFOLDER

win32: LIBS += -lchartdir50
unix:  LIBS += -lchartdir

SOURCES += main.cpp

INCLUDEPATH = $(DEVDIR)/other_libraries/chartdirector/include
