#-------------------------------------------------
#
# Project created by QtCreator 2012-05-25T11:27:51
#
#-------------------------------------------------

QT       += core gui

include($(DEVDIR)/galaxy_warnings.pri)

TARGET = config_keys_poc
TEMPLATE = app


SOURCES += main.cpp\
    gex_line_edit.cpp \
    tree_model_completer.cpp \
    db_keys_editor.cpp \
    regexp_validator.cpp

HEADERS  += \
    gex_line_edit.h \
    tree_model_completer.h \
    db_keys_editor.h \
    regexp_validator.h

FORMS    += \
    db_keys_editor.ui

OTHER_FILES +=
