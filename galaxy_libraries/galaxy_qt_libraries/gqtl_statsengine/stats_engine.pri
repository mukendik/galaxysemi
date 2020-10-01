
SOURCES += \
    sources/stats_algo.cpp \
    sources/r_algo.cpp \
    sources/mv_groups_builder.cpp \
    sources/mv_outliers_finder.cpp \
    sources/stats_data.cpp \
    sources/r_data.cpp \
    sources/stats_engine.cpp \
    sources/r_engine.cpp \
    sources/r_matrix.cpp \
    sources/r_vector.cpp \
    sources/r_protected_object.cpp \
    sources/r_object.cpp \
    sources/g_sexp.cpp \
    sources/shape_identifier.cpp

HEADERS += \
    ../include/mv_groups_builder.h \
    ../include/mv_outliers_finder.h \
    ../include/stats_engine.h \
    ../include/r_matrix.h \
    ../include/r_vector.h \
    ../include/r_object.h \
    ../include/stats_algo.h \
    ../include/stats_data.h \
    ../include/r_algo.h \
    ../include/r_data.h \
    sources/r_engine.h \
    sources/r_protected_object.h \
    sources/g_sexp.h \
    sources/r_object_private.h \
    sources/stats_algo_private.h \
    sources/r_algo_private.h \
    sources/stats_data_private.h \
    ../include/shape_identifier.h \

RESOURCES += \
    $$PWD/resources/scripts.qrc

