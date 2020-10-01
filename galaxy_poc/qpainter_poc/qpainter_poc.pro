QT += core
QT += gui
QT += svg

unix: QT += network

TARGET = qpainter_poc
TEMPLATE = app
CONFIG += console
DEFINES += PAINTER_AS_SERVICE
#DEFINES += PAINTER_USING_QCOREAPP
#DEFINES += PAINTER_USING_QGUIAPP

DESTDIR = $(DEVDIR)/galaxy_poc/qpainter_poc

include($(DEVDIR)/galaxy_common.pri)

SOURCES += \
    main.cpp

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(QTSRCDIR)

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

LIBS += -l$$gexTargetName(gqtl_service) \
        -l$$gexTargetName(gqtl_svg) \
#  -l$$gexTargetName(gqtl_log) \
