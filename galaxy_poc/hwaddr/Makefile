# ---------------------------------------------------------------------------- #
# Makefile pour hwaddr
# ---------------------------------------------------------------------------- #
#
# H=gexprod@cirith-ungol-prod make send
# H=gexprod@dunharrow-prod    make send
# H=gexprod@durthang-prod     make send
# H=gexprod@barad-eithel-prod make send

PROJET = hwaddr
OBJETS = hwaddr.o
CFLAGS = -O2 -Wall -Wextra -Werror -g

PROG = $(PROJET)

.SUFFIXES: .c

.PHONY: all
all: $(PROG)

$(PROG): $(OBJETS)
	$(CC) $^ -o $@

.PHONY: test
test: $(PROG)
	@$(TRACER) ./$(PROG)

.PHONY: clean
clean:
	$(RM) $(OBJETS) $(PROG) *~

.PHONY: send
send:
	rsync --checksum -ti $(MAKEFILE_LIST) *.c \
	$(H):/home/gexprod/$(PROJET)/

$(OBJETS): $(MAKEFILE_LIST)
