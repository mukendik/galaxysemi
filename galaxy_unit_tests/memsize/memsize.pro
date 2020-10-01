TARGET = memsize
SOURCES = memsize.cpp
#
# needed to link with gqtl because using some GUI Qt classes
QT += widgets
# to see printf
CONFIG += console

include($(DEVDIR)/galaxy_common.pri)
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
# LIBS += $$GALAXY_LIBSPATH $$GALAXY_LIBS
LIBS += -l$$gexTargetName(gqtl) -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gqtl) -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)
win32: LIBS += -lpsapi
