#-------------------------------------------------
#
# Project created by QtCreator 2012-06-06T11:15:46
#
#-------------------------------------------------

QT += core gui script
QT += widgets

include($(DEVDIR)/galaxy_common.pri)

DEPENDPATH +=  $(DEVDIR)/galaxy_products/gex_product/gex-pb/include

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/gex-pb/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include/

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gqtl)
LIBS += -L$(DEVDIR)/galaxy_products/gex_product/gex-pb/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gexpb)
LIBS += -L$(DEVDIR)/other_libraries/qtpropertybrowser-2.5_1-commercial/lib/$$OSFOLDER

win32: LIBS += -lpsapi

TARGET = gexpb_unittest
TEMPLATE = app

# auto-importing has been activated without --enable-auto-import specified on the command line.
# -WI, -enable-auto-import
# QMAKE_LDFLAGS += -WI,-enable-auto-import
# QMAKE_LFFLAGS += -WI,-enable-auto-import

SOURCES += main.cpp\
        mainwindow.cpp
#SOURCES += $(DEVDIR)/galaxy_products/gex_product/gex-log/include/libgexlog_public.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
