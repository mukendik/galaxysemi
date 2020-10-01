DEFINES += MODULE=GQTL_SVG

QT			= core gui
DEFINES		+= QT_BUILD_SVG_LIB
DEFINES		+= QT_NO_USING_NAMESPACE
CONFIG		+= debug_and_release
TEMPLATE	= lib

win32:CONFIG += shared

! linux-g++-64:message( using QTSRCDIR $(QTSRCDIR) ...)

include($(DEVDIR)/galaxy_common.pri)

TARGET		= $$gexTargetName(gqtl_svg)
DESTDIR		= $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
#DLLDESTDIR	= $(DEVDIR)/galaxy_products/gex_product/bin

# MinGW windres is given an error (windres: preprocessing failed) without any root cause
# Make sure the cybwin windres is used (check PATH or remove MinGW windres.exe)
RC_FILE = gqtl_svg.rc

QT         = core-private gui-private
qtHaveModule(widgets): QT += widgets-private

DEFINES   += QT_NO_USING_NAMESPACE
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

linux*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

win32:!shared:CONFIG += static
win32:!static: DEFINES+=QT_MAKEDLL
!static:PRL_EXPORT_DEFINES += QT_SHARED

DEFINES *= QT_NO_CAST_TO_ASCII QT_ASCII_CAST_WARNINGS
DEFINES *= QT_MOC_COMPAT  # we dont need warnings from calling moc code in our
                          # generated code
DEFINES *= QT_USE_FAST_OPERATOR_PLUS QT_USE_FAST_CONCATENATION

win32:DEFINES+=_USE_MATH_DEFINES

HEADERS += \
  sources/qsvggraphics_p.h        \
  sources/qsvghandler_p.h         \
  sources/qsvgnode_p.h            \
  sources/qsvgstructure_p.h       \
  sources/qsvgstyle_p.h           \
  sources/qsvgfont_p.h            \
  sources/qsvgtinydocument_p.h    \
  sources/qsvgrenderer.h          \
  sources/qsvgwidget.h            \
  sources/qgraphicssvgitem.h      \
  sources/qsvggenerator.h


SOURCES += \
      sources/qsvggraphics.cpp        \
      sources/qsvghandler.cpp         \
      sources/qsvgnode.cpp            \
      sources/qsvgstructure.cpp       \
      sources/qsvgstyle.cpp           \
      sources/qsvgfont.cpp            \
      sources/qsvgtinydocument.cpp    \
      sources/qsvgrenderer.cpp        \
      sources/qsvgwidget.cpp          \
      sources/qgraphicssvgitem.cpp    \
      sources/qsvggenerator.cpp

wince*: {
    SOURCES += \
        qsvgfunctions_wince.cpp
    HEADERS += \
        qsvgfunctions_wince_p.h
}

INCLUDEPATH += $(QTSRCDIR)
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
#    $(QTSRCDIR) \
#    $(QTSRCDIR)/../include/QtCore \
#    $(QTSRCDIR)/../include/QtGui \
#    $(QTSRCDIR)/../include

#unix {
#  INCLUDEPATH += $(QTSRCDIR)/../include/QtGui \
#    $(QTSRCDIR)/../include/QtCore
#}

#macx|darwin-g++ {
#  INCLUDEPATH += $(QTSRCDIR)/../include/QtGui \
#    $(QTSRCDIR)/../include/QtCore
#}

#contains(QT_VERSION, ^5) {
#  include($(QTSRCDIR)/qtbase/src/3rdparty/zlib_dependency.pri)
#} else {
#  include($(QTSRCDIR)/3rdparty/zlib_dependency.pri)
#}

contains(QT_CONFIG, system-zlib) {
    if(unix|win32-g++*):     LIBS_PRIVATE += -lz
    else:                    LIBS += zdll.lib
} else {
    INCLUDEPATH += $$[QT_INSTALL_HEADERS/get]/QtZlib
}

CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

OTHER_FILES += \
    gqtl_svg.rc

LIBS += -l$$gexTargetName(gstdl)
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

#GALAXY_LIBS += -lz

#unix {
#	QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $$DLLDESTDIR
#}

