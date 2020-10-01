
isEmpty(OSFOLDER) {

OSFOLDER = undefined

win32:OSFOLDER = win32
cygwin-g++:OSFOLDER = win32
win64:OSFOLDER = win64
win32:{
  contains(QMAKE_HOST.arch, x86_64) {
  OSFOLDER = win64
 }
}
linux-g++:OSFOLDER = linux32
linux-g++-32:OSFOLDER = linux32
linux-g++-64:OSFOLDER = linux64
solaris-g++|solaris-g++-32:OSFOLDER = solaris32
solaris-g++-64:OSFOLDER = solaris64
macx|darwin-g++:OSFOLDER = mac
}

QT       -= core
QT       -= gui

INCLUDEPATH += ../include

LIBS += -L$(DEVDIR)/other_libraries/R/lib/$$OSFOLDER

LIBS += -lR
LIBS += -lRblas

DEFINES += OSFOLDER=$$OSFOLDER

win32: { DEFINES+= R_HOME_DEF=win
} else {
  DEFINES += R_HOME_DEF=$$OSFOLDER
}

TARGET = unit_test_1
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += debug_and_release

TEMPLATE = app

SOURCES += main.cpp
