TARGET = gs_gtl_traceability_ut1
SOURCES = main.cpp

INCLUDEPATH += ../../../include

include($(DEVDIR)/galaxy_common.pri)
LIBS +=\
 -L$(DEVDIR)/galaxy_libraries/galaxy_std_libraries/lib/$$OSFOLDER\
 -lgs_gtl_traceability\
 -lgs_data\

unix
{
  LIBS += -lsqlite3
}
