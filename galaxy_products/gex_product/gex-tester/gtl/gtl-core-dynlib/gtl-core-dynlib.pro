DEFINES += MODULE=GTL

QT -= core gui network

include($(DEVDIR)/galaxy_common.pri)

CONFIG += debug_and_release

# Add QMAKE_LFLAGS +=  --enable-stdcall-fixup ?????
#win32:QMAKE_LFLAGS += --enable-stdcall-fixup -Wl,libgtl.def

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER

DEF_FILE = $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/gtl-core-dynlib/gtl-in.def

# let's profile using gprof ?
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_CXXFLAGS += -pg

# .def useful to generate a .lib on each win target system (ie VS 2010) for linking with target binaries
win32:QMAKE_LFLAGS += -Wl,--output-def,$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/gtl.def
cygwin-g++:QMAKE_LFLAGS += -Wl,--output-def,$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/gtl.def


# Linking with gcc -static -static-libgcc -static-libstdc++ and you won't need libgcc_s_dw2-1.dll and libstdc++-6.dll
win32: LIBS += -static-libstdc++ -static-libgcc -static -lpthread

# DLLDESTDIR: enables to copy the dll in the desired location on win
# Win release: use DLLDESTDIR to copy the dll directly in the location for the GTM package (bin/gtl..),
# because the COPY_FILE tool is not cross-platform
#CONFIG(release, debug|release) {
#    DLLDESTDIR	= $(DEVDIR)/galaxy_products/gex_product/bin/gtl/lib/$$OSFOLDER
#}

INCLUDEPATH += ../include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
contains(DEFINES, GTL_PROFILER_OFF): LIBS += -l$$gexTargetName(gtl_core_noprof)
!contains(DEFINES, GTL_PROFILER_OFF): LIBS += -l$$gexTargetName(gtl_core)
LIBS += -l$$gexTargetName(gstdl)
LIBS += -L../lib/$$OSFOLDER
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
win32:LIBS += libwsock32 libws2_32 libole32 libsnmpapi
# liboleaut32

TARGET = $$gexTargetName(gtl)
contains(DEFINES, GTL_PROFILER_OFF): TARGET = $$gexTargetName(gtl_noprof)

# VERSION = 3.6 # add version to name of lib : example: gtl3.dll
QMAKE_TARGET_COMPANY = GalaxySemi
QMAKE_TARGET_PRODUCT = GTL
QMAKE_TARGET_DESCRIPTION = GalaxyTesterLibrary
QMAKE_TARGET_COPYRIGHT = GalaxySemi

TEMPLATE = lib

DEFINES += GTLCOREDYNLIB_LIBRARY

SOURCES += gtlcoredynlib.cpp

HEADERS += gtlcoredynlib.h \
        gtl-core-dynlib_global.h \
        ../include/gtl_core.h
# just to force gtl dyn lib to relink if gtlcore has changed. Does not seem to work anymore with Qt5...
contains(DEFINES, GTL_PROFILER_OFF): PRE_TARGETDEPS += ../lib/$$OSFOLDER/lib$$gexTargetName(gtl_core_noprof).a
!contains(DEFINES, GTL_PROFILER_OFF): PRE_TARGETDEPS += ../lib/$$OSFOLDER/lib$$gexTargetName(gtl_core).a
PRE_TARGETDEPS += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER/lib$$gexTargetName(gstdl).a

! linux-g++-64:message($$PRE_TARGETDEPS)

# Unix release: copy the dll in the location for the GTM package (bin/gtl..),
# Not needed on Windows, where this is taken care of by DLLDESTDIR.
#CONFIG(release, debug|release) {
#unix {
#    QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $(DEVDIR)/galaxy_products/gex_product/bin/gtl/lib/$$OSFOLDER
#}
#}

# In case we want to generate a .lib file to be used for linking the DLL with linkers others than mingw on win
#CONFIG(release, debug|release) {
#win32 {
#    # Use dlltool to generate a .lib file that can be used with VS.
#    # The generated .lib works with VS 6 on WinNT, bu for safety, the .lib is not delivered with th package.
#    # Instead, we deliver a .bat the customer can use to generate his .lib on his specific platform.
#    QMAKE_POST_LINK = dlltool -d $(DEVDIR)/galaxy_products/gex_product/bin/gtl/lib/$$OSFOLDER/gtl.def \
#    --dllname gtl.dll --output-lib \
#    $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER/gtl.lib --kill-at
#}
#}

OTHER_FILES += gtl-dynlib.rc
OTHER_FILES += $(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/gtl-core-dynlib/gtl-in.def
# MinGW windres is given an error (windres: preprocessing failed) without any root cause
# Make sure the cybwin windres is used (check PATH or remove MinGW windres.exe)
RC_FILE = gtl-dynlib.rc

#
#win32: LIBS += -L$(DEVDIR)/other_libraries/EProfiler/lib/win32 -lEProfiler.dll
