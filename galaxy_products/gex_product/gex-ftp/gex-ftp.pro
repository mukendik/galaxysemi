DEFINES += MODULE=GSFTP

TEMPLATE = app
CONFIG += debug_and_release
QT += widgets
QT += network

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_products/gex_product/bin
TARGET = $$gexTargetName(gex-ftp)

SOURCES += sources/main.cpp \
    sources/gexftp_settings_widget.cpp \
    sources/gexftp_settings.cpp \
    sources/gexftp_service_widget.cpp \
    sources/gexftp_servertransfer_widget.cpp \
    sources/gexftp_server.cpp \
    sources/gexftp_passwordconfirmation_dialog.cpp \
    sources/gexftp_page_widget.cpp \
    sources/gexftp_missingfield_dialog.cpp \
    sources/gexftp_mainwindow.cpp \
    sources/gexftp_mainconsole.cpp \
    sources/gexftp_mainbase.cpp \
    sources/gexftp_download.cpp \
    sources/gexftp_core.cpp \
    sources/gexftp_client.cpp \
    sources/gexftp_calendar_dialog.cpp \
    sources/gexftp_qftp.cpp \
    sources/gexftp_browse_dialog.cpp
HEADERS += sources/resource.h \
    sources/gexftp_settings_widget.h \
    sources/gexftp_settings.h \
    sources/gexftp_service.h \
    sources/gexftp_service_widget.h \
    sources/gexftp_servertransfer_widget.h \
    sources/gexftp_server.h \
    sources/gexftp_passwordconfirmation_dialog.h \
    sources/gexftp_page_widget.h \
    sources/gexftp_missingfield_dialog.h \
    sources/gexftp_mainwindow.h \
    sources/gexftp_mainconsole.h \
    sources/gexftp_mainbase.h \
    sources/gexftp_download.h \
    sources/gexftp_core.h \
    sources/gexftp_constants.h \
    sources/gexftp_client.h \
    sources/gexftp_calendar_dialog.h \
    sources/gexftp_qftp.h \
    sources/gexftp_browse_dialog.h
INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/gex-ftp/sources/ui \
    $(QTSRCDIR)/../include/QtCore \
    $(QTSRCDIR)
DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_products/gex_product/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
    $$PWD/sources
RC_FILE = sources/gex_ftp.rc
FORMS += sources/gexftp_settings_widget_base.ui \
    sources/gexftp_service_widget_base.ui \
    sources/gexftp_servertransfer_widget_base.ui \
    sources/gexftp_passwordconfirmation_dialog_base.ui \
    sources/gexftp_page_widget_base.ui \
    sources/gexftp_missingfield_dialog_base.ui \
    sources/gexftp_mainwindow_base.ui \
    sources/gexftp_calendar_dialogbase.ui \
    sources/gexftp_browse_dialog.ui
#UI_DIR += $(DEVDIR)/galaxy_products/gex_product/gex-ftp/sources/ui
RESOURCES += sources/gex_ftp.qrc
#GALAXY_LIBS += -l$$gexTargetName(gqtl_ftp) \
# -l$$gexTargetName(gstdl)\
# -l$$gexTargetName(gqtl)\
# -l$$gexTargetName(gqtl_filelock)\
# -l$$gexTargetName(gqtl_service)


GALAXY_LIBS += -l$$gexTargetName(gstdl) \
    -l$$gexTargetName(gqtl_stdf) \
    -l$$gexTargetName(gqtl_filelock) \
     -l$$gexTargetName(gqtl_ftp) \
    -l$$gexTargetName(gqtl)\
    -l$$gexTargetName(gqtl_service) 
win32:GALAXY_LIBS += -lws2_32 -lgdi32 -lpsapi


GALAXY_LIBSPATH +=\
 -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER \
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

# PLEASE DON'T REMOVE THIS LINE
# user_custom.pri must be used to customize the .pro in your environment.
# Use this in order to do tests which won't be committed on the SCM.
exists(user_custom.pri) {
	include(user_custom.pri)
}

win32:DEFINES += PSAPI_VERSION=1
linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
