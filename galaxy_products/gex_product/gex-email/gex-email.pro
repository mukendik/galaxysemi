# -------------------------------------------------
# Project created by QtCreator 2009-03-25T12:09:59
# -------------------------------------------------

TEMPLATE = app
QT       += core gui
DEFINES += MODULE=GEXEMAIL

QT += network
CONFIG += debug_and_release
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
TARGET = $$gexTargetName(gex-email)

SOURCES += sources/main.cpp \
    sources/gex_email_smtp.cpp \
    sources/gex_email_sendwidget.cpp \
    sources/gex_email_send.cpp \
    sources/gex_email_outlook.cpp \
    sources/gex_email_mainwindow.cpp \
    sources/gex_email_mainconsole.cpp \
    sources/gex_email_mainbase.cpp \
    sources/gex_email_core.cpp \
    sources/gex_connectiontest_dialog.cpp
HEADERS += sources/resource.h \
    sources/gexemail_constants.h \
    sources/gex_email_smtp.h \
    sources/gex_email_service.h \
    sources/gex_email_sendwidget.h \
    sources/gex_email_send.h \
    sources/gex_email_outlook.h \
    sources/gex_email_mainwindow.h \
    sources/gex_email_mainconsole.h \
    sources/gex_email_mainbase.h \
    sources/gex_email_core.h \
    sources/gex_connectiontest_dialog.h
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
                $(DEVDIR)/galaxy_products/gex_product/include \
                $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
        $(DEVDIR)/galaxy_products/gex_product/gex-email/sources/ui
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
  $(DEVDIR)/galaxy_products/gex_product/include \
  $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
  $$PWD/sources
FORMS += sources/smtppage_base.ui \
    sources/outlookpage_base.ui \
    sources/gex_email_mainwindow_base.ui \
    sources/gex_connectiontest_dialog_base.ui
#UI_DIR +=  $(DEVDIR)/galaxy_products/gex_product/gex-email/sources/ui
RESOURCES += sources/gex_email.qrc
RC_FILE = sources/gex_email.rc

GALAXY_LIBS -= -l$$gexTargetName(gexdb_plugin_base)
GALAXY_LIBS += -l$$gexTargetName(gstdl) \
#    -l$$gexTargetName(gqtl_sysutils) \
#    -l$$gexTargetName(gqtl) \
#	-l$$gexTargetName(gstdl_utils_c) \
    -l$$gexTargetName(gqtl_filelock) \
#	-l$$gexTargetName(gstdl_jwsmtp) \
  -l$$gexTargetName(gqtl_service)

GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \
          -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER


CONFIG(debug, debug|release) {
  OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
  OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

LIBS += $$GALAXY_LIBS \
    $$GALAXY_LIBSPATH

win32 {
  LIBS += -lws2_32
}

#PLEASE DON'T REMOVE THIS LINE
#user_custom.pri must be used to customize the .pro in your environment.
#Use this in order to do tests which won't be committed on the SCM.
exists(user_custom.pri) {
  include(user_custom.pri)
}

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
