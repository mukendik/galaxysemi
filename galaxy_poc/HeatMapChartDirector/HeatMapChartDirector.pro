TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += threads
CONFIG += debug_and_release
CONFIG -= qt

SOURCES += \
    main.cpp

include($(DEVDIR)/galaxy_common.pri)
INCLUDEPATH += $(DEVDIR)/other_libraries/chartdirector/include

LIBS += -L$(DEVDIR)/other_libraries/chartdirector/lib/$$OSFOLDER

# ChartDirector
win32: LIBS += -lchartdir50
unix:  LIBS += -lchartdir


