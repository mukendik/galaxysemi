QT += core gui webkit
QT += widgets
QT += webkitwidgets

TARGET = html_dynsize_poc
TEMPLATE = app
CONFIG += console

DESTDIR = $(DEVDIR)/galaxy_poc/html_dynamic_size

include($(DEVDIR)/galaxy_common.pri)

SOURCES += main.cpp

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

#LIBS += -l$$gexTargetName(gqtl_service) \
 #       -l$$gexTargetName(gqtl_svg) \
#  -l$$gexTargetName(gqtl_log) \

OTHER_FILES += \
    no_table.htm \
    ppt_slide.htm

HEADERS += \
    webview.h
