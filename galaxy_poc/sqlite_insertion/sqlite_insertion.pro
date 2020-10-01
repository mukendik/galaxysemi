#-------------------------------------------------
#
# Project created by QtCreator 2013-06-06T10:44:09
#
#-------------------------------------------------

QT       -= core gui

TARGET = sqlite_insertion
CONFIG   += console
CONFIG   -= app_bundle

include($(DEVDIR)/galaxy_warnings.pri)

INCLUDEPATH += sqlite-amalgamation-3071700

TEMPLATE = app


SOURCES += main.cpp \
    sqlite-amalgamation-3071700/sqlite3.c
