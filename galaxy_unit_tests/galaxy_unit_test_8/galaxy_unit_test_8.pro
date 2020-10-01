DEFINES += MODULE=UT8

TEMPLATE = app
CONFIG += console
CONFIG += qt

include($(DEVDIR)/galaxy_common.pri)

PRE_TARGETDEPS += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER/lib$$gexTargetName(gstdl).a

# EProfiler
#QMAKE_CXXFLAGS_DEBUG += -finstrument-functions
#QMAKE_CFLAGS_DEBUG += -finstrument-functions
#QMAKE_LFLAGS += -Wl,-Map=unit_test_3.map
#win32: LIBS += -L$(DEVDIR)/other_libraries/EProfiler/lib/win32 -lEProfiler.dll

INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
LIBS += -l$$gexTargetName(gstdl)

# add these libs at last in order to link...
win32:LIBS += libwsock32 libws2_32 libole32 liboleaut32 libsnmpapi

SOURCES += main.cpp

#message($$LIBS)
