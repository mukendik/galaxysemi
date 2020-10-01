
QT       += core

TARGET = unit_test_shape_identifier
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl_statsengine)

INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas
# LIBS += -L/usr/lib/

# DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin

SOURCES += \
    main.cpp

# uncomment to use gprof
# QMAKE_CFLAGS   += -pg
# QMAKE_CXXFLAGS += -pg
# QMAKE_LFLAGS   += -pg




