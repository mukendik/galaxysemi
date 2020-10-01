#-------------------------------------------------
#
# Project created by QtCreator 2014-08-31T16:30:23
#
#-------------------------------------------------

include($(DEVDIR)/galaxy_common.pri)

QT       += core

QT       -= gui

TARGET = gstdl_hashtable_ut1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

PRE_TARGETDEPS += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER/lib$$gexTargetName(gstdl).a

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)

SOURCES += main.c
