DEFINES += MODULE=QGLV

#-------------------------------------------------
#
# Project created by QtCreator 2012-06-06T10:30:50
#
#-------------------------------------------------

QT       += core gui xml opengl

TARGET = qglviewer_unittest
TEMPLATE = app

include($(DEVDIR)/galaxy_common.pri)

SOURCES += main.cpp\
        mainwindow.cpp

SOURCES += $(DEVDIR)/galaxy_products/gex_product/gex/sources/drill_3d_viewer.cpp
HEADERS  += $(DEVDIR)/galaxy_products/gex_product/gex/sources/drill_3d_viewer.h

INCLUDEPATH += $(DEVDIR)/other_libraries/libqglviewer/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex/sources
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

LIBS += -L$(DEVDIR)/other_libraries/libqglviewer/lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

LIBS += -l$$gexTargetName(gstdl)
win32: LIBS += -l$$gexTargetName(qglviewer)2
unix:  LIBS += -l$$gexTargetName(qglviewer)
macx:  LIBS += -l$$gexTargetName(qglviewer)

linux-g++*: LIBS += -lGLU
linux-g++-*: LIBS += -ldl   # needed for some sqlite3.o versions

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
