QT -= core gui xml opengl
QT += script

TARGET = galaxy_ut_js_regexp
TEMPLATE = app

CONFIG += console
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

SOURCES += main.cpp 
