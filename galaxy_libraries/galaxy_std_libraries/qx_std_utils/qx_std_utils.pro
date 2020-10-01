#-------------------------------------------------
#
# Project created by QtCreator 2018-07-17T14:54:25
#
#-------------------------------------------------

QT = core

TARGET = qx_std_utils
TEMPLATE = lib

include($(DEVDIR)/galaxy_common.pri)

DEFINES += QX_STD_UTILS_API

# look into general defines to rule how the library is built
contains(DEFINES, QX_SHARED_API) : {
    CONFIG += shared
} else : {
    CONFIG += static
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    sources/string_utils/case_insensitive_char_traits.cpp \
    sources/string_utils/string_manipulations.cpp

HEADERS += \
    sources/qx_std_utils_api.h \
    sources/string_utils/case_insensitive_char_traits.h \
    sources/string_utils/string_manipulations.h

INCLUDEPATH += sources

RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER
TARGET = $$gexTargetName(qx_std_utils)
