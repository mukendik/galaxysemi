SOURCES+= sources/report_options.cpp \
	sources/report_options_reset.cpp \
	sources/report_options_type_definition.cpp \
	sources/gex_options_map.cpp \
	sources/gex_options_handler.cpp

HEADERS+= sources/report_options.h \
	sources/report_options_defines.h \
	sources/report_options_type_definition.hpp \
	sources/gex_options_map.h \
	sources/gex_options_handler.h

OTHER_FILES+= $(DEVDIR)/galaxy_products/gex_product/gex/sources/xml/gex_options.xml
