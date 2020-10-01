include($(DEVDIR)/galaxy_warnings.pri)

TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

include(../../src/qtpropertybrowser.pri)
# Input
HEADERS += qtcanvas.h mainwindow.h
SOURCES += qtcanvas.cpp mainwindow.cpp main.cpp

