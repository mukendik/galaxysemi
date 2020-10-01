# ---------------------------------------------------------------------------- #
# Makefile for ut_gs_buffer
# ---------------------------------------------------------------------------- #
PROJECT  = ut_gs_buffer
INCPATH  = -I../../../include
CXXFLAGS = $(INCPATH)
ifneq "$(MINGWDIR)" ""
  PROJECT := $(PROJECT).exe
endif
$(shell mkdir -p bin)

.PHONY: all
all: bin/$(PROJECT)

bin/$(PROJECT): main.o ../../gs_buffer.o
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: bin/$(PROJECT)
	@bin/$(PROJECT) && echo ok

.PHONY: vtest
vtest: bin/$(PROJECT)
	@valgrind --leak-check=full --show-leak-kinds=all -q \
	bin/$(PROJECT) && echo ok

include ../../../include/gs_common.mk
