#-------------------------------------------------
#
# Project created by QtCreator 2013-10-11T10:38:46
#
#-------------------------------------------------

QT       += core script scripttools

include($(DEVDIR)/galaxy_warnings.pri)

TARGET = poc-pat-js
TEMPLATE = app


SOURCES += main.cpp \
    JSClasses.cpp

HEADERS  += JSClasses.h

# For EmbeddedProfiler
#QMAKE_CXXFLAGS += -finstrument-functions
#QMAKE_LFLAGS   += -Wl,-Map=SE-Poc.map

#LIBS += -lEProfiler.dll -LG:/Tools/development/Profilers/embeddedProfilers/EProfiler/windows32-mingw-intel/lib

#QMAKE_POST_LINK = g:/Tools/development/Profilers/embeddedProfilers/PerformanceAnalyzer/windows32-mingw-intel/bin/EProfilerSymGen.exe \
#                  ./SE-Poc.map ./SE-Poc.sym
