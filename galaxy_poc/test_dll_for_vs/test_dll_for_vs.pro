QT -= core gui

TEMPLATE = lib
CONFIG += debug_and_release
CONFIG += dll

include($(DEVDIR)/galaxy_warnings.pri)

DEFINES += TEST_EXPORTS

# Add QMAKE_LFLAGS +=  --enable-stdcall-fixup ?????
#win32:QMAKE_LFLAGS += --enable-stdcall-fixup -Wl,libgtl.def
# Seems to be useful to make the dll usable with MSVC :
win32:QMAKE_LFLAGS += -Wl,--output-def,test_dll_for_vs.def

TARGET = test_dll_for_vs

SOURCES = test_dll_for_vs.c

HEADERS = test_dll_for_vs.h

win32:QMAKE_POST_LINK = dlltool -d test_dll_for_vs.def --dllname test_dll_for_vs.dll --output-lib test_dll_for_vs.lib --kill-at
