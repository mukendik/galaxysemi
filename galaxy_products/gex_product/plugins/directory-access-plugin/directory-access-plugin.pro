TEMPLATE = subdirs
QT += widgets

DEFINES += MODULE=GEXDA

CONFIG += debug_and_release

include($(DEVDIR)/galaxy_warnings.pri)

SUBDIRS += da-galaxy

