

#INCLUDEPATH = \
#    $(DEVDIR)/other_libraries/R/include

#LIBS += \
#    -L$(DEVDIR)/other_libraries/R/lib/win32 -lR

SOURCES += \
    stats_algo.cpp \
    r_algo.cpp \
    mv_groups_builder.cpp \
    mv_outliers_finder.cpp \
    stats_data.cpp \
    r_data.cpp \
    stats_engine.cpp \
    r_engine.cpp \
    sources/mv_pat_tests.cpp

HEADERS += \
    stats_algo.h \
    r_algo.h \
    mv_groups_builder.h \
    mv_outliers_finder.h \
    stats_data.h \
    r_data.h \
    stats_engine.h \
    r_engine.h

