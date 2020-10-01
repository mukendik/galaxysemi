#-------------------------------------------------
#
# Project created by QtCreator 2013-05-06T11:27:39
#
#-------------------------------------------------

QT       += core network

QT       -= gui

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtc


TARGET = multithreaded_sockets
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gtl_core)
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)

SOURCES += main.cpp \
    server.cpp \
    client.cpp

HEADERS += \
    main.h
