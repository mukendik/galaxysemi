# ---------------------------------------------------------------------------- #
# Makefile common.mk
# ---------------------------------------------------------------------------- #
COBJECTS    = $(patsubst %.c,%.o,$(wildcard *.c))
CXXOBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
WARNINGS    = -Wall -Wextra -Werror -O1 -D_FORTIFY_SOURCE=2
CFLAGS     += $(WARNINGS)
CXXFLAGS   += $(WARNINGS)
CDEP        = $(patsubst %.o,%.d,$(COBJECTS))
CXXDEP      = $(patsubst %.o,%.d,$(CXXOBJECTS))
ifeq "$(DEBUG)" "1"
  CFLAGS   += -g
  CXXFLAGS += -g
endif
ifneq "$(MINGWDIR)" ""
  CC        = gcc
  RM        = del
  CFLAGS   += -posix
  CXXFLAGS += -posix
else
  CFLAGS   += -fPIC
  CXXFLAGS += -fPIC
endif

.SUFFIXES: .c .cpp

.PHONY: dep
dep:
	@for f in $(patsubst %.d,%,$(CDEP)); do \
	    $(CC) $(CFLAGS) -MM -MF $$f.d $$f.c; \
	    cat $$f.d; \
	done
	@for f in $(patsubst %.d,%,$(CXXDEP)); do \
	    $(CXX) $(CXXFLAGS) -MM -MF $$f.d $$f.cpp; \
	    cat $$f.d; \
	done

.PHONY: clean
clean:
	@$(RM) *.o *~

.PHONY: mrproper
mrproper: clean
	@$(RM) $(PROJECT) *.d *.so

.PHONY: cppcheck
cppcheck:
	@cppcheck --quiet --enable=all --suppressions cppcheck.txt --std=posix \
	$(INCPATH) .

.PHONY: tar
tar:
	@cd .. && \
	tar cvzf $(PROJECT).tgz \
	--exclude=*~ \
	--exclude=*.o \
	--exclude=*.so \
	--exclude=*.d \
	--exclude=bin \
	--exclude=.*.swp \
	--exclude=release \
	--exclude=debug \
	--exclude=Makefile_linux* \
	include/gs_*.mk include/gs_*.h gs_data gs_gtl_traceability


-include $(CDEP) $(CXXDEP)

$(COBJECTS) $(CXXOBJECTS): $(MAKEFILE_LIST)
