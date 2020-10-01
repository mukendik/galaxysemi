# ---------------------------------------------------------------------------- #
# Makefile for json-c
# ---------------------------------------------------------------------------- #
# zypper install libjson-c-devel libjson-c2-debuginfo
# ---------------------------------------------------------------------------- #
PROJECT = json-c
OBJECTS = json-c.o
CFLAGS  = -g -O1 -D_FORTIFY_SOURCE=2 -Wall -Wextra -Werror -I/usr/include/json-c
LDFLAGS = -ljson-c

PROG = $(PROJECT)

.SUFFIXES: .c

.PHONY: all
all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(PROG)
	@$(TRACER) ./$(PROG)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROG) *~

$(OBJECTS): $(MAKEFILE_LIST)
