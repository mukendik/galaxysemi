DEFINES += MODULE=GQTL_STDF

TEMPLATE = lib
CONFIG += staticlib debug_and_release

include($(DEVDIR)/galaxy_common.pri)

DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(gqtl_stdf)

SOURCES += \
    sources/stdfparse.cpp \
    sources/stdfrecord.cpp \
    sources/stdfrecords_v4.cpp \
    sources/stdfrecords_v3.cpp \
    sources/stdf_content_utils.cpp \
    sources/stdf_head_and_site_number_decipher.cpp

HEADERS += \
    ../include/stdfparse.h \
    ../include/stdfrecord.h \
    ../include/stdfrecords_v4.h \
    ../include/stdfrecords_v3.h \
    ../include/stdf_content_utils.h \
    ../include/stdf_head_and_site_number_decipher.h

LIBS += -l$$gexTargetName(gstdl) -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER \
        -l$$gexTargetName(gqtl) -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

INCLUDEPATH = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
            $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
#            $(DEVDIR)/galaxy_products/gex_product/gex-log/include

DEPENDPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
            $(DEVDIR)/galaxy_products/gex_product/gex-log/include \
            $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include \
            $$PWD/sources

CONFIG(debug, debug|release) {
        OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
        OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR

linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64
