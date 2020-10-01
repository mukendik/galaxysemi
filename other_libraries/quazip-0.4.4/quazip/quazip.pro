include($(DEVDIR)/galaxy_common.pri)
macx-g++ | win32-g++ {
    include($(QTSRCDIR)/qtbase/src/3rdparty/zlib.pri)
}

TARGET = $$gexTargetName(quazip)

DESTDIR = $(DEVDIR)/other_libraries/quazip-0.4.4/lib/$$OSFOLDER

TEMPLATE = lib
CONFIG += qt
QT -= gui
DEPENDPATH += .
INCLUDEPATH += .

macx {
    QMAKE_CFLAGS_WARN_ON += -Wno-typedef-redefinition
    QMAKE_CFLAGS_WARN_ON += -Wno-parentheses-equality
}

macx-clang {
    INCLUDEPATH += /usr/include
} else {
    INCLUDEPATH += $(QTSRCDIR)/qtbase/src/3rdparty/zlib
}

DEFINES += QUAZIP_BUILD
CONFIG(staticlib): DEFINES += QUAZIP_STATIC

# Input
HEADERS += \
    crypt.h\
    ioapi.h\
    JlCompress.h\
    quaadler32.h\
    quachecksum32.h\
    quacrc32.h\
    quazip.h\
    quazipfile.h\
    quazipfileinfo.h\
    quazipnewinfo.h\
    quazip_global.h\
    unzip.h\
    zip.h\

SOURCES += *.c *.cpp

unix:!symbian {
    headers.path=$$PREFIX/include/quazip
    headers.files=$$HEADERS
    #target.path=$$PREFIX/lib
    #INSTALLS += headers target
    OBJECTS_DIR=.obj
    MOC_DIR=.moc

}

# Seems z library link is not required on Windows. Is it required on unix?
# LIBS+= -lz

win32 {
    headers.path=$$PREFIX/include/quazip
    headers.files=$$HEADERS
    # target.path=$$PREFIX/lib
    # INSTALLS += headers target
}

macx-clang {
    LIBS += -lz
}

symbian {

    # Note, on Symbian you may run into troubles with LGPL.
    # The point is, if your application uses some version of QuaZip,
    # and a newer binary compatible version of QuaZip is released, then
    # the users of your application must be able to relink it with the
    # new QuaZip version. For example, to take advantage of some QuaZip
    # bug fixes.

    # This is probably best achieved by building QuaZip as a static
    # library and providing linkable object files of your application,
    # so users can relink it.

    CONFIG += staticlib
    CONFIG += debug_and_release

    LIBS += -lezip

    #Export headers to SDK Epoc32/include directory
    exportheaders.sources = $$HEADERS
    exportheaders.path = quazip
    for(header, exportheaders.sources) {
        BLD_INF_RULES.prj_exports += "$$header $$exportheaders.path/$$basename(header)"
    }
}
