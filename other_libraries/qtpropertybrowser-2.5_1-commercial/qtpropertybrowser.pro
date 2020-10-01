TEMPLATE=subdirs
CONFIG += ordered

include($(DEVDIR)/galaxy_warnings.pri)

include(common.pri)
qtpropertybrowser-uselib:SUBDIRS=buildlib
#SUBDIRS+=examples
linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64# test unit
#SUBDIRS+=examples
