
include($(DEVDIR)/galaxy_common.pri)
QT       += core webkit network gui printsupport webkitwidgets

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = html_auto_table_font
TEMPLATE = app

win32: LIBS += -lws2_32 -lgdi32 -lsnmpapi


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h
