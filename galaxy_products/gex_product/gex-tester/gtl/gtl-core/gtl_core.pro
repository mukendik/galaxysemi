
DEFINES += MODULE=GTL

QT -= core gui network

#DEFINES += NO_GTM
#DEFINES += DUMP_FOR_NO_GTM
DEFINES += GTL_LOG_OPENCLOSE_TIME

TEMPLATE = lib
CONFIG += staticlib debug_and_release

include($(DEVDIR)/galaxy_common.pri)

DEFINES += GTLCORE_MODULE

# DEFINES += 'SYSLOG_CONF_DIR=""'

# let s profile
#QMAKE_CXXFLAGS_DEBUG += -finstrument-functions
#QMAKE_CFLAGS_DEBUG += -finstrument-functions
#QMAKE_LFLAGS += -Wl,-Map=gtl.map
#win32: LIBS += -L$(DEVDIR)/other_libraries/EProfiler/lib/win32 -lEProfiler.dll

QMAKE_CFLAGS_WARN_ON = -w  # suppr warnings from sqlite3.c ?

#QMAKE_CFLAGS += -gstabs+ -pg
#QMAKE_CFLAGS_DEBUG += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_STRIP =
#QMAKE_STRIP = $${CROSS_COMPILE}strip
#QMAKE_STRIPFLAGS_LIB += --strip-unneeded

#DESTDIR = $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
# It seems qmake generates the AR script (object_script.libgtl_cored.Debug/Release) differently
# when there are less or more than 7 or 8 files to be put in the lib.
# But $(DEVDIR) is not usable inside AR script.
# let s try to use relative path ...
DESTDIR = ../lib/$$OSFOLDER

TARGET = $$gexTargetName(gtl_core)
contains(DEFINES, GTL_PROFILER_OFF): TARGET = $$gexTargetName(gtl_core_noprof)

SOURCES += ../../gtc/gtc_netmessage.c \
    gtl_error.c \
    gtl_main.c \
    gtl_message.c \
    gtl_server.c \
    gtl_socket.c \
    gtl_testlist.c \
    gtl_getset.c \
    gtl_output.c \
    gtl_output_sqlite.c \
    gtl_init.c \
    gtl_log.c \
    gtl_processmessage.c \
    gtl_bin.c \
#    gtl_output_sql.c \
    gtl_httpserver.c \
    gtl_close.c \
    gtl_beginjob.c \
    gtl_syslog.cpp \ # test for cpp migration and gqtl syslog functions
    gtl_test.c \
    gtl_retest.c \
    gtl_open.c \
    gtl_command.c \
    gtl_splitlot.c


SOURCES += gtl_dump.cpp

# as we are now linking libwinpthread morover in static
SOURCES += gtl_profile.cpp

CONFIG(debug, debug|release) {
    SOURCES += gtl_output_stdf.cpp
}

SOURCES += $(DEVDIR)/other_libraries/sqlite/3.7.17/sqlite3.c
# on winNT _strtoi64 is unfindable...
CONFIG(debug, debug|release) {
  DEFINES += GTLDEBUG
}

SOURCES += $(DEVDIR)/other_libraries/mongoose/mongoose-3.7/mongoose.c

HEADERS += ../../gtc/gtc_constants.h \
        ../../gtc/gtc_netmessage.h \
    gtl_constants.h \
    gtl_error.h \
    gtl_message.h \
    gtl_server.h \
    gtl_socket.h \
    gtl_testlist.h \
    ../include/gtl_core.h \
    gtl_getset.h \
    gtl_output.h \
    gtl_profile.h

# Define how to create version.h
# version.target = sql_create_script.h
# version.commands = $$replace(SQL,
# version.depends = .git
#QMAKE_EXTRA_TARGETS += version
#PRE_TARGETDEPS += version.h

# is RC_FILE Qt project only and compatible with C libraries ?
# RC_FILE = ./tdr_embedded.sql

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
        $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/include \
        $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtc \
# BG: re-added following line to fix compilation error. Will, please check...
        $(DEVDIR)/other_libraries/syslog-win32/include \
        $(DEVDIR)/other_libraries/sqlite/3.7.17 \
        $(DEVDIR)/other_libraries/mongoose/mongoose-3.7 \
        $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/sources \
        $(DEVDIR)/galaxy_products/gex_product/plugins/gexdb_plugin_galaxy/install/sqlite

#        $(DEVDIR)/other_libraries/sqlite/3.7.15.1 \


LIBS += -l$$gexTargetName(gstdl) \
        -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

# test to link with msvcrt static
# win32: LIBS += -lmsvcrt -lstdc++  : unusefull
win32: LIBS += -static-libstdc++

# todo : add pre deps on gstdl
PRE_TARGETDEPS +=  $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER/lib$$gexTargetName(gstdl).a

#INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

#DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
#	$$PWD/sources

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

# Qt creates for ressources some .cpp files and for the moment GTL is C only...
#RESOURCES += res.qrc
