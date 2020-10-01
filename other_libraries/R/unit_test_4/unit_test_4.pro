#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T08:13:19
#
#-------------------------------------------------

QT       += core

TARGET = unit_test_4
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl_statsengine)

#LIBS +=  -lmingw32 -lPsapi -lsnmpapi

INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas

# LIBS += -L/usr/lib/
# R
#win32: LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER
#linux-g++-32 {
#    LIBS += -L/usr/lib/R/lib -lR -lRblas
#}
#linux-g++-64 {
#    LIBS += -L/usr/lib64/R/lib  -lR -lRblas
#}
#macx-clang {
#    LIBS += -L/Library/Frameworks/R.framework/Versions/Current/Resources/lib
#}



#DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin


SOURCES += \
    main.cpp
