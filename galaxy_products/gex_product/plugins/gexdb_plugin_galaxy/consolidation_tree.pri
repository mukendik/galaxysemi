# sources
SOURCES +=  sources/consolidation_tree.cpp \
    sources/consolidation_tree_replies.cpp \
    sources/consolidation_tree_validator.cpp \
    sources/consolidation_tree_test_condition.cpp \
    sources/consolidation_tree_consolidation_rule.cpp \
    sources/consolidation_tree_query_engine.cpp \
    sources/consolidation_tree_query_engine_p.cpp \
    sources/consolidation_tree_query_filter.cpp \
    sources/consolidation_tree_commands.cpp \
    sources/consolidation_tree_period.cpp \
    sources/consolidation_tree_updater.cpp \
    sources/consolidation_tree_data.cpp

# headers
HEADERS += sources/consolidation_tree_replies.h \
    sources/consolidation_tree.h \
    sources/consolidation_tree_validator.h \
    sources/consolidation_tree_test_condition.h \
    sources/consolidation_tree_consolidation_rule.h \
    sources/consolidation_tree_query_engine.h \
    sources/consolidation_tree_defines.h \
    sources/consolidation_tree_query_engine_p.h \
    sources/consolidation_tree_query_filter.h \
    sources/consolidation_tree_commands.h \
    sources/consolidation_tree_period.h \
    sources/consolidation_tree_updater.h \
    sources/consolidation_tree_data.h

OTHER_FILES += sources/resources/consolidation_tree.xsd
