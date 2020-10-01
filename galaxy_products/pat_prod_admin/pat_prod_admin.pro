include($(DEVDIR)/galaxy_warnings.pri)

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
TARGET = pat_prod_admin
QT = core \
    gui
TEMPLATE = app

SOURCES += main.cpp \
    patprodrecipe_users.cpp \
    patprodrecipe_recipes.cpp \
    user_profile.cpp \
    group_profile.cpp \
    patprodrecipe_groups.cpp \
    patprod_admin.cpp \
    recipe_profile_eng.cpp \
    recipe_profile_prod.cpp \
	../pat_prod_recipe/flow_entry.cpp \
    prod_recipe_file.cpp \
    ../pat_prod_recipe/patprodrecipe.cpp

HEADERS += patprodrecipe_Admin.h \
    user_profile.h \
    group_profile.h \
    recipe_profile_eng.h \
    recipe_profile_prod.h \
    prod_recipe_file.h \
    ../pat_prod_recipe/patprodrecipe.h
FORMS += user_profile.ui \
    group_profile.ui \
    patprodrecipe_Admin.ui \
    recipe_profile_ENG.ui \
    recipe_profile_PROD.ui \
    ../pat_prod_recipe/patprodrecipe.ui \
    ../pat_prod_recipe/flow_entry.ui
RESOURCES += pat_prod_admin.qrc
