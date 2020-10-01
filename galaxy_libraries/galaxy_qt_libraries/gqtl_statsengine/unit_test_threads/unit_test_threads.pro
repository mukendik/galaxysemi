#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T08:13:19
#
#-------------------------------------------------


QT       += core
TARGET   = unit_test_threads
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
LIBS        += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl_statsengine)

INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
LIBS        += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas

SOURCES += \
    main.cpp \
    statsthread.cpp

HEADERS += \
    statsthread.h
