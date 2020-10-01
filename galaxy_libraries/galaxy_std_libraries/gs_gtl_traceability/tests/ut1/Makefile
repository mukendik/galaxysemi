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
BRANCH   = master
INCPATH  = ../../../include
LIBPATH  = ../../../lib/linux64
CFLAGS   = -I$(INCPATH)
CXXFLAGS = -I$(INCPATH)
LDFLAGS  = -lgs_data -lgs_gtl_traceability -L$(LIBPATH)
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

ifeq "$(F)" ""
  F = $(HOME)/prod/gex-prod-qa-$(BRANCH)/gex-prod-tests/000220.scenario/in-ref/FT_ROOM_MAX8556ETE+_TNA0KA104AA_ETS88-016A_20131102100046_A.sqlite
endif
ifeq "$(GEX_PATH)" ""
  GEX_PATH = $(HOME)/prod/gex-prod-$(BRANCH)/galaxy_products/gex_product/bin
endif

.PHONY: all
all: bin/$(PROJECT)

bin/$(PROJECT): main.o gs_buffer.o
	$(CXX) $^ $(LDFLAGS) -o $@

gs_buffer.o: ../../../gs_data/gs_buffer.c
	$(CC) $(CFLAGS) $^ -c -o $@

.PHONY: test
test: bin/$(PROJECT)
	@LD_LIBRARY_PATH=$(LIBPATH) \
	DYLD_LIBRARY_PATH=$(LIBPATH) \
	PATH=$(PATH):$(LIBPATH) \
	GEX_PATH=../../../../../galaxy_products/gex_product/bin \
	bin/$(PROJECT) $(F) && echo ok

.PHONY: vtest
vtest: bin/$(PROJECT)
	@LD_LIBRARY_PATH=$(LIBPATH) \
	GEX_PATH=../../../../../galaxy_products/gex_product/bin \
	valgrind --leak-check=full --show-leak-kinds=all -q \
	--suppressions=valgrind.supp \
	bin/$(PROJECT) $(F) && echo ok

include ../../../include/gs_common.mk
