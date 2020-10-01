MODULE=GQTL_SE

TEMPLATE = lib
CONFIG += shared
QT       += core
QT       += widgets

CONFIG += debug_and_release
#CONFIG -= app_bundle
CONFIG += console

include($(DEVDIR)/galaxy_common.pri)
GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)
include(stats_engine.pri)

# -Wno-format-zero-length : To Do : how to add compiler options command line
TARGET = $$gexTargetName(gqtl_statsengine)

#
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

DEFINES += R_NO_REMAP=1
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_statsengine/sources
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

win32 {
    INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
    LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR
}
linux-g++-32 {
#    INCLUDEPATH += /usr/lib/R/include
#    LIBS += -L/usr/lib/R/lib -lR -lRblas
    INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
    LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas
}
linux-g++-64 {
#    INCLUDEPATH += /usr/lib64/R/include
#    LIBS += -L/usr/lib64/R/lib -lR -lRblas
    INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
    LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER -lR -lRblas
}
macx {
    INCLUDEPATH += $(DEVDIR)/other_libraries/R/include
    LIBS += -L$(DEVDIR)/other_libraries/R/lib/mac -lR -lRblas
}

LIBS += $$GALAXY_LIBSPATH
LIBS += $$GALAXY_LIBS

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR


