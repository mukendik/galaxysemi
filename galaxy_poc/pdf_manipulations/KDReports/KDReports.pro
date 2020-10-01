#-------------------------------------------------
#
# Project created by QtCreator 2016-01-21T09:51:47
#
#-------------------------------------------------
QT       += core gui widgets printsupport xml

TARGET = KDReport
TEMPLATE = app

INCLUDEPATH += $$PWD/src

SOURCES += src/main.cpp\
           src/mainwindow.cpp

HEADERS  += src/mainwindow.h

FORMS    += ui/mainwindow.ui

RESOURCES += resources/resources.qrc

# this directory contains build scripts for third party libraries
SCRIPT_DIR = $$PWD/scripts

# generic flags to build
# functionnal for Qt-5.4.2 or greater
greaterThan( QT_MAJOR_VERSION, 4 ) {
    greaterThan( QT_MINOR_VERSION, 3 ) {
        QMAKE_CXXFLAGS += -std=c++14
        CONFIG += c++14
    }
}

# this is the directory containing KDReport sources
THIRDPARTY_DIR = $$PWD/thirdparty
KDREPORTS_DIR = $$THIRDPARTY_DIR/KDReports

INCLUDEPATH += $$KDREPORTS_DIR/include/KDReports

# extract qmake bin directory name
QMAKE_EXECUTABLE = $$QMAKE_QMAKE
QMAKE_BIN_DIR = $$dirname( QMAKE_EXECUTABLE )

macx {
    # specify which c++ lib to use
    QMAKE_CXXFLAGS += -stdlib=libc++

    # clean adds, applyable for all build configuration
    QMAKE_CLEAN += -rf \
                   $$KDREPORTS_DIR/bin \
                   $$KDREPORTS_DIR/lib \
                   $$KDREPORTS_DIR/include

    # debug configuration
    CONFIG( debug, debug|release ) {
        # configure KDReport before its build
        system( $$SCRIPT_DIR/configure_kdreports.sh \
                $$PWD \
                $$QMAKE_BIN_DIR \
                debug )

        # occurs before the link step
        QMAKE_PRE_LINK += $$SCRIPT_DIR/build_kdreports.sh \
                            $$PWD \
                            debug \
                            8
    }

    # release configuration
    CONFIG( release, release|debug) {
        # configure KDReport before its build
        system( $$SCRIPT_DIR/configure_kdreports.sh \
                $$PWD \
                $$QMAKE_BIN_DIR \
                release )

        # occurs before the link step
        QMAKE_PRE_LINK += $$SCRIPT_DIR/build_kdreports.sh \
                            $$PWD \
                            release \
                            8
    }

    LIBS += -L$$KDREPORTS_DIR/lib -lkdreports
}

unix:!macx {}

win32 {}
