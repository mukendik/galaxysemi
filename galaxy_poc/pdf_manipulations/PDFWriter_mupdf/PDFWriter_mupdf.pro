#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T11:07:06
#
#-------------------------------------------------
QT       += core gui widgets

TARGET = PDFWriter_mupdf
TEMPLATE = app

INCLUDEPATH += $$PWD/src

SOURCES += src/main.cpp\
           src/mainwindow.cpp \
    src/result_array.cpp

HEADERS  += src/mainwindow.h \
    src/result_array.h

FORMS    += ui/mainwindow.ui

RESOURCES += resources/resources.qrc

# update all thirdparty libraries
SCRIPT_DIR = $$PWD/scripts

system( $$SCRIPT_DIR/update_submodules.sh $$PWD )

# generic flags to build
# functionnal for Qt-5.4.2 or greater
greaterThan( QT_MAJOR_VERSION, 4 ) {
    greaterThan( QT_MINOR_VERSION, 3 ) {
        QMAKE_CXXFLAGS += -std=c++14
        CONFIG += c++14
    }
}

# define third party stuff
THIRD_DIR = $$PWD/thirdparty
PDF_WRITER_DIR = $$THIRD_DIR/PDF-WRITER
MUPDF_DIR = $$THIRD_DIR/mupdf

# specify supplementary header directories for thirdparty libraries
INCLUDEPATH += $$PDF_WRITER_DIR \
               $$PDF_WRITER_DIR/FreeType/include

INCLUDEPATH += $$MUPDF_DIR/include

# specific mac os x configuration
macx {
    # trick to delete directories
    QMAKE_CLEAN += -rf \
                   $$PWD/doc/output.pdf \
                   $$PWD/doc/output.png \

    # specify which c++ lib to use
    QMAKE_CXXFLAGS += -stdlib=libc++

    # debug configuration
    CONFIG( debug, debug|release ) {
        QMAKE_CLEAN += $$PDF_WRITER_DIR/build/debug/
        QMAKE_CLEAN += $$MUPDF_DIR/build/debug/

        QMAKE_PRE_LINK = $$SCRIPT_DIR/build_thirdparty_libraries.sh \
                           $$PWD \
                           $$QMAKE_CXX \
                           $$QMAKE_CC \
                           debug \
                           8

        LIBS += -L$$PDF_WRITER_DIR/build/debug/FreeType -lFreeType
        # the LibJpeg used in PDFWriter is older than the one used in mupdf.
        # As the most recent LibJpeg is included in libmupdfthird.a, this one
        # become unnecessary
        # LIBS += -L$$PDF_WRITER_DIR/build/debug/LibJpeg -lLibJpeg
        LIBS += -L$$PDF_WRITER_DIR/build/debug/LibTiff -lLibTiff
        LIBS += -L$$PDF_WRITER_DIR/build/debug/PDFWriter -lPDFWriter
        LIBS += -L$$PDF_WRITER_DIR/build/debug/ZLib -lZlib

        LIBS += -L$$MUPDF_DIR/build/debug -lmupdfthird -lmupdf
    }
    # release configuration
    CONFIG( release, release|debug ){
        QMAKE_CLEAN += $$PDF_WRITER_DIR/build/release/
        QMAKE_CLEAN += $$MUPDF_DIR/build/release/

        QMAKE_PRE_LINK = $$SCRIPT_DIR/build_thirdparty_libraries.sh \
                           $$PWD \
                           $$QMAKE_CXX \
                           $$QMAKE_CC \
                           release \
                           8

        LIBS += -L$$PDF_WRITER_DIR/build/release/FreeType -lFreeType
        # the LibJpeg used in PDFWriter is older than the one used in mupdf.
        # As the most recent LibJpeg is included in libmupdfthird.a, this one
        # become unnecessary
        # LIBS += -L$$PDF_WRITER_DIR/build/release/LibJpeg -lLibJpeg
        LIBS += -L$$PDF_WRITER_DIR/build/release/LibTiff -lLibTiff
        LIBS += -L$$PDF_WRITER_DIR/build/release/PDFWriter -lPDFWriter
        LIBS += -L$$PDF_WRITER_DIR/build/release/ZLib -lZlib

        LIBS += -L$$MUPDF_DIR/build/release -lmupdfthird -lmupdf
    }

}

# specific unix configuration
unix:!macx {
}

# specific windows configuration
win32 {
}
