include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

include(../../src/qtpropertybrowser.pri)
# Input
HEADERS += objectcontroller.h
SOURCES += objectcontroller.cpp main.cpp

