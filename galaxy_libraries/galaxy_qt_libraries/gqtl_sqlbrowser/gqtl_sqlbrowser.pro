DEFINES += MODULE="SQLBROWSER"

QT       = core gui sql
#DEFINES	+= QT_NO_USING_NAMESPACE
DEFINES += LIB_LIBRARY
CONFIG += console
CONFIG  += debug_and_release
TEMPLATE = lib

include($(DEVDIR)/galaxy_common.pri)
QT += widgets

TARGET		= $$gexTargetName(gqtl_sqlbrowser)
DESTDIR		= $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
#DLLDESTDIR	= $(DEVDIR)/galaxy_products/gex_product/bin

#RC_FILE = gqtl_svg.rc

win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x66000000
solaris-cc*:QMAKE_CXXFLAGS_RELEASE -= -O2

linux*:QMAKE_LFLAGS += $$QMAKE_LFLAGS_NOUNDEF

#win32:!shared:CONFIG += static
win32:!static: DEFINES+=QT_MAKEDLL
!static:PRL_EXPORT_DEFINES += QT_SHARED

#DEFINES *= QT_NO_CAST_TO_ASCII QT_ASCII_CAST_WARNINGS
DEFINES *= QT_MOC_COMPAT #we don t need warnings from calling moc code in our generated code
DEFINES *= QT_USE_FAST_OPERATOR_PLUS QT_USE_FAST_CONCATENATION

win32:DEFINES+=_USE_MATH_DEFINES

INCLUDEPATH += $(QTSRCDIR)
INCLUDEPATH += sources
INCLUDEPATH += ../include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

unix {
	INCLUDEPATH += $(QTSRCDIR)/../include/QtGui \
		$(QTSRCDIR)/../include/QtCore
}

CONFIG(debug, debug|release) {
	OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
	OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

#unix {
#	QMAKE_POST_LINK = $(COPY_FILE) $(DESTDIR)/*$$TARGET* $$DLLDESTDIR
#}

HEADERS += \
    sources/qsqlconnectiondialog.h \
    sources/connectionwidget.h \
    sources/browser.h \
    ../include/gqtl_sqlbrowser.h \
    sources/textedit.h

SOURCES += \
    sources/qsqlconnectiondialog.cpp \
    sources/connectionwidget.cpp \
    sources/browser.cpp \
    sources/gqtl_sqlbrowser.cpp \
    sources/textedit.cpp

FORMS += \
    sources/browserwidget.ui \
    sources/qsqlconnectiondialog.ui

LIBS += -l$$gexTargetName(gstdl)
LIBS += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
