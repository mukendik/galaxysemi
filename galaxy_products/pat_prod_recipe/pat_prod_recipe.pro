include($(DEVDIR)/galaxy_warnings.pri)

DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += $(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/include \
    $(DEVDIR)/galaxy_libraries/galaxy_std_libraries/include
TARGET = pat_prod_recipe
QT = core \
    gui \
    widgets
TEMPLATE = app
SOURCES += main.cpp \
    ../pat_prod_admin/prod_recipe_file.cpp \
    flow_entry.cpp
SOURCES += patprodrecipe.cpp
HEADERS += patprodrecipe.h \
    prod_recipe_file.h \
    ../pat_prod_admin/prod_recipe_file.h
FORMS += patprodrecipe.ui \
    flow_entry.ui
RESOURCES += pat_prod_recipe.qrc
