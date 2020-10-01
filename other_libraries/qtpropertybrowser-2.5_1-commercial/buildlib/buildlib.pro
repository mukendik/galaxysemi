TEMPLATE=lib
CONFIG += qt dll qtpropertybrowser-buildlib shared
mac:CONFIG += absolute_library_soname
#win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all
CONFIG += debug_and_release

include($(DEVDIR)/galaxy_warnings.pri)

! linux-g++-64:message(CONFIG: $$CONFIG)include(../src/qtpropertybrowser.pri)
TARGET = $$QTPROPERTYBROWSER_LIBNAME
DESTDIR = $$QTPROPERTYBROWSER_LIBDIR
win32 {
#    DLLDESTDIR = $$[QT_INSTALL_BINS]
    QMAKE_DISTCLEAN += $$[QT_INSTALL_BINS]\\$${QTPROPERTYBROWSER_LIBNAME}.dll
}
target.path = $$DESTDIR
INSTALLS += target
linux-g++*:DEFINES += _FILE_OFFSET_BITS=64
solaris-g++:DEFINES += _FILE_OFFSET_BITS=64

macx-clang{
    QMAKE_CXXFLAGS_WARN_ON -= -Werror
}