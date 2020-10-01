#-------------------------------------------------
#
# Project created by QtCreator 2014-03-05T11:25:36
#
#-------------------------------------------------

#QT       -= gui

TEMPLATE = lib

CONFIG += debug_and_release

include($(DEVDIR)/galaxy_common.pri)

TARGET = $$gexTargetName(gqtl_patcore)
DESTDIR = $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

DEFINES += GQTL_PATCORE_LIBRARY
DEFINES += MODULE=GQTL_PATCORE

INCLUDEPATH += $(DEVDIR)/galaxy_products/gex_product/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/include
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/gqtl_patcore/sources
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include

LIBS += $$GALAXY_LIBS
LIBS += $$GALAXY_LIBSPATH

SOURCES += \
    sources/pat_global.cpp \
    sources/pat_mv_rule.cpp \
    sources/pat_definition.cpp \
    sources/pat_options.cpp \
    sources/pat_rules.cpp \
    sources/pat_recipe_private.cpp \
    sources/pat_recipe_io.cpp \
    sources/pat_recipe.cpp \
    sources/pat_recipe_io_csv.cpp \
    sources/pat_recipe_io_json.cpp \
    sources/pat_dynamic_limits.cpp \
    sources/pat_option_reticle.cpp

HEADERS +=\
        sources/gqtl_patcore_global.h \
    include/pat_global.h \
    include/pat_defines.h \
    include/gex_pat_constants_extern.h \
    include/gex_pat_constants.h \
    include/pat_mv_rule.h \
    include/pat_definition.h \
    include/pat_options.h \
    include/pat_rules.h \
    sources/pat_recipe_private.h \
    include/pat_recipe_io_json.h \
    include/pat_recipe_io_csv.h \
    include/pat_recipe_io.h \
    include/pat_recipe.h \
    include/pat_dynamic_limits.h \
    include/pat_option_reticle.h

CONFIG(debug, debug|release) {
    OBJECTS_DIR = $$PWD/debug/$$OSFOLDER
}
else {
    OBJECTS_DIR = $$PWD/release/$$OSFOLDER
}

# Set the temporary directory for MOC files and RCC files to the same place than the OBJECTS files
RCC_DIR = $$OBJECTS_DIR
MOC_DIR = $$OBJECTS_DIR
