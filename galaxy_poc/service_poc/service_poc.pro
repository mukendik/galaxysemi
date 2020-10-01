#-------------------------------------------------
#
# Project created by QtCreator 2013-05-16T12:28:26
#
#-------------------------------------------------

QT       += core xml

QT       -= gui
QT       += webkit
QT += webkitwidgets

CONFIG   += console
CONFIG   -= app_bundle
DEFINES += MODULE=SERVICE_POC
TEMPLATE = app
DESTDIR = $(DEVDIR)/galaxy_poc/service_poc/bin

include($(DEVDIR)/galaxy_common.pri)

TARGET = $$gexTargetName(service_poc)

SOURCES += \
    sources/main.cpp \
    sources/my_service.cpp \
    sources/my_core_task.cpp \
    sources/my_event.cpp

HEADERS += \
    sources/my_service.h \
    sources/my_core_task.h \
    sources/my_event.h


INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

GALAXY_LIBS += -l$$gexTargetName(gqtl_service)
GALAXY_LIBS -= -l$$gexTargetName(gtl_core)
GALAXY_LIBS -= -l$$gexTargetName(gstdl)
GALAXY_LIBS -= -l$$gexTargetName(gqtl)
GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)


GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
        $$GALAXY_LIBSPATH

