isEmpty(OSFOLDER) {

CONFIG *= debug_and_release

OSFOLDER = undefined

win32:OSFOLDER = win32
cygwin-g++:OSFOLDER = win32
win64:OSFOLDER = win64
win32:{
  contains(QMAKE_HOST.arch, x86_64) {
  OSFOLDER = win64
 }
}
linux-g++:OSFOLDER = linux32
linux-g++-32:OSFOLDER = linux32
linux-g++-64:OSFOLDER = linux64
solaris-g++|solaris-g++-32:OSFOLDER = solaris32
solaris-g++-64:OSFOLDER = solaris64
macx|darwin-g++:OSFOLDER = mac

DEFINES += QT_NO_CAST_FROM_BYTEARRAY

# DO ONLY UNCOMMENT THE FOLLOWING DEFINE IF YOU NEED TO GENERATE A SPECIFIC GTL WITHOUT GTL PROFILING EMBEDDED
# (GTL_PROFILE_START and GTL_PROFILE_STOP empty).
# THIS WILL GENERATE gtl_core_noprof.a, gtl_noprof.dll (ut_noprof.exe for unit_test_1).
# THIS SHOULD BE NECESSARY FOR INTERNAL TESTING ONLY. FOR PACKAGES DELIVERED TO OUR CUSTOMERS, IF WE WANT THE
# POSSIBILITY TO DISABLE THE PROFILER, WE'LL HAVE TO IMPLEMENT A GTL KEY (e.g. "gtl_profiler"="on" or "off"),
# which will add the overhead of a if statement for profiler start and stop.
#DEFINES += GTL_PROFILER_OFF

defineReplace(gexTargetName){
  unset(GALAXY_TARGET_NAME)
  GALAXY_TARGET_NAME = $$1
  CONFIG(debug, debug|release) {
    RET = $$member(GALAXY_TARGET_NAME, 0)d
  }
  isEmpty(RET):RET = $$GALAXY_TARGET_NAME
  return($$RET)
 }

win32 {
  # Fix QTBUG-21878
  #QMAKE_LFLAGS += -Wl,--enable-auto-import
  # in order to control order of libs, add OS libs at the very end of GALAXY_LIBS
  #LIBS += -lws2_32 -lsnmpapi -lPsapi
  #GALAXY_LIBS += -lsnmpapi -lPsapi -lgdi32 -ladvapi32 -lws2_32

  # Fix Exceptions and Dynamiv_cast
  #QMAKE_CXXFLAGS += -fexceptions -frtti  # do not work
#  CONFIG += exceptions rtti
#  !staticlib:CONFIG += dll

  # Fix QRCC error
  QMAKE_RCC = $$[QT_INSTALL_BINS]$${DIR_SEPARATOR}rcc.exe
}
# add this lib for all project linking with gqtl, i.e. nearly all
# snmpapi : Simple Network Management Protocol api
# psapi : process status api
# Psapi is now needed for GetProcessMemoryInfo. Todo : add Psapi.dll to the packages ?
# gqtl lib must be before psapi in order to link successfully on win32
GALAXY_LIBS += -l$$gexTargetName(gqtl)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_qt_libraries/lib/$$OSFOLDER

# plugin_base must be before -lws2_32
GALAXY_LIBS += -l$$gexTargetName(gexdb_plugin_base)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_products/gex_product/plugins/lib/$$OSFOLDER

# gtl_core must be before -lws2_32 (really ?)
GALAXY_LIBS += -l$$gexTargetName(gtl_core)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_products/gex_product/gex-tester/gtl/lib/$$OSFOLDER
# gstdl do needs ws2 ? before or after ?
GALAXY_LIBS += -l$$gexTargetName(gstdl)
GALAXY_LIBSPATH += -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER

win32:GALAXY_LIBS += -lsnmpapi -lpsapi -lws2_32 -lkernel32

win32-g++: QMAKE_CFLAGS += -posix
win32-g++: QMAKE_CXXFLAGS += -posix

include($(DEVDIR)/galaxy_warnings.pri)

exists(user_custom.pri) {
    include(user_custom.pri)
}

win*-msvc* {
  CONFIG += no_batch
  QMAKE_CXXFLAGS += /MP /FS
  QMAKE_CXXFLAGS -= /O2 /O1 -O1 -O2
  QMAKE_CXXFLAGS_DEBUG -= /O2 /O1 -O1 -O2
  QMAKE_CXXFLAGS_RELEASE -= /O1 -O1 -O2
  QMAKE_CXXFLAGS_RELEASE += /O2
}

macx-clang {
  QMAKE_CXXFLAGS += -stdlib=libc++
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
  DEFINES += GL_SILENCE_DEPRECATION
}

# Specify that libraries using this defined constant will be built as shared object, by not defining this one,
# library checking this constant will be built as static object and incorporated in the linked object. Libraries that
# are using this defined constant are :
# - qx_bin_mapping
# - qx_std_utils
DEFINES += QX_SHARED_API
}
