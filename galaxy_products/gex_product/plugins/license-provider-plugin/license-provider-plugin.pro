message(ERROR: no longer use because the clean doesn"'"t work)

include($(DEVDIR)/galaxy_common.pri)

TEMPLATE = subdirs
QT += widgets

DEFINES += MODULE=GEXLP

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER -l$$gexTargetName(gstdl)

CONFIG += ordered debug_and_release
SUBDIRS += base-lp
SUBDIRS += gs-lp
SUBDIRS += fnp-lp
win32: SUBDIRS += ter-lp
