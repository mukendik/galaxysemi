
include($(DEVDIR)/galaxy_common.pri)

QT       += core
# in order to use gqtl
QT       += gui
QT       += widgets

INCLUDEPATH += ../include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include

LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER -l$$gexTargetName(gqtl)

LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER

LIBS += -lR

#contains(QMAKE_COMPILER_DEFINES, _WIN64) {
#  DEFINES += _OS_=win
#} else {
#  DEFINES += _OS_=$$OSFOLDER
#}

DEFINES += OSFOLDER=$$OSFOLDER

win32: { DEFINES+= R_HOME_DEF=win
} else {
  DEFINES += R_HOME_DEF=$$OSFOLDER
}

TARGET = unit_test_1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

win32:LIBS += libwsock32 libws2_32 -lPsapi -lsnmpapi
