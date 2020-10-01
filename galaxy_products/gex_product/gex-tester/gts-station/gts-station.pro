DEFINES += MODULE=GTS

QT += network
QT += widgets
TEMPLATE = app
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)
GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)

#DEFINES += NO_GTM
#DEFINES += BENCHMARK
#DEFINES += DYNGTL

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
TARGET = $$gexTargetName(gts-station)

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

SOURCES	+= main.cpp \
	gts_station_gtlcommandsdialog.cpp \
	gts_station_infowidget.cpp \
	gts_station_mainwindow.cpp \
	gts_station_messagedialog.cpp \
	gts_station_newlotdialog.cpp \
	gts_station_objects.cpp \
	gts_station_outputwidget.cpp \
	gts_station_setupwidget.cpp \
  gts_station_statswidget.cpp \
    gts_station_gtlwidget.cpp
#  ../../gex-lm/sources/read_system_info.cpp
#    ../../gex/sources/stdfrecord.cpp \
#    ../../gex/sources/stdfparse.cpp \
#    ../../gex/sources/stdf.cpp\
#    ../../gex-log/include/libgexlog_public.cpp
HEADERS += \
	gts_station_gtlcommandsdialog.h \
	gts_station_infowidget.h \
	gts_station_mainwindow.h \
	gts_station_messagedialog.h \
	gts_station_newlotdialog.h \
	gts_station_objects.h \
	gts_station_outputwidget.h \
	gts_station_setupwidget.h \
  gts_station_statswidget.h \
    gts_station_gtlwidget.h
#    ../../gex/sources/stdfrecords_v4.h \
#    ../../gex/sources/stdfparse.h \
#    ../../gex/sources/stdf.h
#The following line was changed from FORMS to FORMS3 by qt3to4
FORMS	= \
	gts_station_statswidget_base.ui \
	gts_station_infowidget_base.ui \
	gts_station_outputwidget_base.ui \
	gts_station_messagedialog_base.ui \
	gts_station_gtlcommandsdialog_base.ui \
	gts_station_newlotdialog_base.ui \
	gts_station_setupwidget_base.ui \
        gts_station_gtlwidget.ui \
        gts_station_mainwindow.ui
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
        $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
        $(DEVDIR)/galaxy_products/gex_product/gex/sources \
        $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include
#        $(DEVDIR)/galaxy_products/gex_product/gex-log/include

#PRE_TARGETDEPS += ../gtl/lib/$$OSFOLDER/lib$$gexTargetName(gtl_core).a
DEPENDPATH += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include

contains(DEFINES, DYNGTL){
  # dyn libs : gtl.a and gtl.dll/so/dylib
  LIBS += -l$$gexTargetName(gtl)
  WIN32: PRE_TARGETDEPS += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/$$gexTargetName(gtl).dll
}
else {
  # static lib : gtl_core.a
  LIBS +=  -l$$gexTargetName(gtl_core)
}

LIBS +=\
 -l$$gexTargetName(gqtl_stdf)\
 $$GALAXY_LIBS\
 $$GALAXY_LIBSPATH\
 -l$$gexTargetName(gqtl_datakeys)\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER\
 -L$(DEVDIR)/galaxy_products/gex_product/bin -lgs_data\

win32 {
    LIBS += -lws2_32 -lsnmpapi
}

linux-g++-*: LIBS += -ldl

CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

RESOURCES += \
    gts-station.qrc
