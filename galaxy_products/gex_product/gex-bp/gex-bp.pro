QT += network
QT += gui
QT += widgets

TARGET		= gex
CONFIG		+= console

include($(DEVDIR)/galaxy_warnings.pri)

SOURCES		= src/npgex.cpp

DLLDESTDIR += plugins

# DLLDESTDIR += "c:/Users/will/AppData/Roaming/Mozilla/Firefox/Profiles/d6jjul7r.default/extensions/william.tambellini@galaxysemi.com/plugins"

win32 {
# On Windows, uncomment the following lines to build a plugin that can
# be used also in Internet Explorer, through ActiveX.
	CONFIG += qaxserver
	LIBS += -lurlmon

   RC_FILE		= npgex.rc
} else:mac {
   QMAKE_INFO_PLIST = Info.plist
   REZ_FILES += grapher.r
   rsrc_files.files = grapher.rsrc
   rsrc_files.path = Contents/Resources
   QMAKE_BUNDLE_DATA += rsrc_files
}

    HEADERS += sources/gqtl_qftp.h \

include(qtbrowserplugin/qtbrowserplugin.pri)
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \

OTHER_FILES += \
    npgex.rc
