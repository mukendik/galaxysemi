exists(config.pri):infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qtpropertybrowser-uselib
include($(DEVDIR)/galaxy_common.pri)TEMPLATE += fakelib
QTPROPERTYBROWSER_LIBNAME = $$qtLibraryTarget(QtSolutions_PropertyBrowser-2.5)
TEMPLATE -= fakelib
QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/$$OSFOLDER
unix:qtpropertybrowser-uselib:!qtpropertybrowser-buildlib:QMAKE_RPATHDIR += $$QTPROPERTYBROWSER_LIBDIR
