# ---------------------------------------------------------------------------- #
# Makefile for POCs
# ---------------------------------------------------------------------------- #
# SUBDIRS = $(shell find . -type d | cut -c3-)
SUBDIRS = \
gsmalloc \
hwaddr \
lssqldrivers \
testfpu \
testsql \
memsize \
glx \
xdr \
rserve \
qscriptengine \

.PHONY: $(SUBDIRS)

.SUFFIXES:

.PHONY: all
all:

.PHONY: clean
clean:
	$(RM) *~
	@for dir in $(SUBDIRS); do \
		echo "$$dir: "; \
		$(MAKE) --no-print-directory -C $$dir clean; \
	done
