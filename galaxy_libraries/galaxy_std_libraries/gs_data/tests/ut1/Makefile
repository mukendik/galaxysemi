# ---------------------------------------------------------------------------- #
# Makefile for ut1
# ---------------------------------------------------------------------------- #
# export MINGWDIR=/cygdrive/c/TDM-GCC-64-4.8.1
# export PATH=$MINGWDIR/bin:$PATH
# mingw32-make dep
# mingw32-make
# make test F=c:/cygwin/home/gexprod/tmp/.sqlite
# ---------------------------------------------------------------------------- #
PROJECT  = ut1
INCPATH  = -I../../../include
CXXFLAGS = $(INCPATH)
LDFLAGS  = -L../.. -lgs_data
ifneq "$(MINGWDIR)" ""
  PROJECT := $(PROJECT).exe
  LIBEXT = dll
else
  LDFLAGS += -ldl
  ifeq "$(shell uname)" "Darwin"
    LIBEXT = dylib
  else
    LIBEXT = so
  endif
endif
$(shell mkdir -p bin)

.PHONY: all
all: bin/$(PROJECT)

bin/$(PROJECT): main.o ../../libgs_data.$(LIBEXT)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: bin/$(PROJECT)
	@LD_LIBRARY_PATH=../.. \
	DYLD_LIBRARY_PATH=../.. \
	PATH=$(PATH):../.. \
	bin//$(PROJECT) $(F) && echo ok

.PHONY: vtest
vtest: bin/$(PROJECT)
	@LD_LIBRARY_PATH=../.. \
	valgrind --leak-check=full --show-leak-kinds=all -q \
	--suppressions=valgrind.supp \
	bin/$(PROJECT) $(F) && echo ok

include ../../../include/gs_common.mk
