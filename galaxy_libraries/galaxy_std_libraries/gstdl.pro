QT -= core gui

TEMPLATE = lib
CONFIG += staticlib debug_and_release

QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport
DEFINES -= UNICODE

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $$PWD/lib/$$OSFOLDER
TARGET = $$gexTargetName(gstdl)

SOURCES += \
    gstdl_utils/sources/gstdl_utils.cpp \
    gstdl_utils/sources/gstdl_systeminfo_debug.cpp \
    gstdl_utils/sources/gstdl_systeminfo.cpp \
    gstdl_utils/sources/gstdl_safefile.cpp \
    gstdl_utils/sources/gstdl_membuffer.cpp \
    gstdl_utils/sources/gstdl_inibuffer.cpp \
    gstdl_utils/sources/gstdl_filemap.cpp \
    gstdl_utils/sources/gstdl_csvfile.cpp \
    gstdl_utils/sources/gstdl_crypto.cpp \
    gstdl_utils/sources/gstdl_linreg.cpp \
    gstdl_blowfish_c/sources/gstdl_blowfish.c \
    gstdl_errormgr/sources/gstdl_errormgr.cpp \
    gstdl_info/sources/gstdl_info.cpp \
    gstdl_jwsmtp/sources/gstdl_mailer.cpp \
    gstdl_jwsmtp/sources/gstdl_compat.cpp \
    gstdl_jwsmtp/sources/gstdl_base64.cpp \
    gstdl_stdf/sources/stdf.cpp \
    gstdl_stdf/sources/gstdl_stdftype.cpp \
#    gstdl_stdf/sources/gstdl_stdfdll.cpp \
    gstdl_utils_c/sources/gstdl_utils_c.c \
    gstdl_utils_c/sources/gstdl_netmessage_c.c \
    gstdl_utils_c/sources/gstdl_neterror_c.c \
    gstdl_utils_c/sources/gstdl_netbuffer_c.c \
    gstdl_utils_c/sources/gstdl_md5checksum_c.c \
    gstdl_utils_c/sources/gstdl_binfiles_c.c \
    gstdl_utils_c/sources/gstdl_ringbuffer_c.c \
    gstdl_jwsmtp/sources/gstdl_ntp.cpp \
#    gstdl_utils_c/sources/gstdl_cpuinfo.c \
#   gstdl_utils_c/sources/gstdl_hashtable_c.c

win32 {
SOURCES += \
    gstdl_utils/sources/gstdl_utilsdll.cpp \
    gstdl_utils/sources/gstdl_hostid_snmpapi.cpp \
    gstdl_utils/sources/gstdl_hostid_mibaccess.cpp \
    gstdl_errormgr/sources/gstdl_errormgrdll.cpp \
    gstdl_info/sources/gstdl_infodll.cpp \
}

HEADERS += gstdl_utils/sources/gstdl_utilsdll.h \
    gstdl_utils/sources/gstdl_systeminfo_debug.h \
    include/gstdl_systeminfo.h \
    gstdl_utils/sources/gstdl_safefile.h \
    gstdl_utils/sources/gstdl_inibuffer.h \
    include/gstdl_hostid_mibaccess.h \
    gstdl_utils/sources/gstdl_csvfile.h \
    include/gstdl_utils.h \
    include/gstdl_linreg.h \
    include/gstdl_filemap.h \
    include/gstdl_crypto.h \
    include/gstdl_membuffer.h \
    gstdl_info/sources/gstdl_infodll.h \
    gstdl_info/sources/gstdl_info.h \
    gstdl_stdf/sources/gstdl_stdfdll.h \
    gstdl_utils_c/sources/gstdl_md5checksumdefines_c.h \
    gstdl_utils_c/sources/gstdl_binfiles_c.h \
    include/gstdl_ringbuffer_c.h \
    include/gstdl_stdffile.h \
    include/stdf.h \
    include/stdf_common.h \
    include/gstdl_type.h \
    include/gs_types.h

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
	$$PWD/sources

CONFIG(debug, debug|release) {
	OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
    # add this define to get some printf logs
    # DEFINES += GSTDL_DEBUG
}
else {
	OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
