# ---------------------------------------------------------------------------- #
# Makefile for rserve server in debug mode
# ---------------------------------------------------------------------------- #
# cd /data
# ls -l Rserve_1.8-0.tar.gz
# tar xzf Rserve_1.8-0.tar.gz
# cd Rserve
# PATH=/usr/bin:$PATH ./configure  # (RPREFIX = /usr)
# cd src
# ln -sf RS-debug.mk Makefile
# make
# make install
# /usr/bin/R CMD Rserve.dbg --RS-socket /tmp/RS-socket
# ---------------------------------------------------------------------------- #
PROJECT = Rserve.dbg
RPREFIX = /usr
OBJECTS = $(subst Rserv.o,,$(patsubst %.c,%.o,$(wildcard *.c)))
RHOME = $(shell $(RPREFIX)/bin/R RHOME)
CFLAGS = -std=gnu99 -DSTANDALONE_RSERVE -DRSERV_DEBUG -DNODAEMON -DNDEBUG\
 -fpic -pthread -g -O2 -Wall -Wextra -DRSERVE_PKG -Iinclude -I$(RHOME)/include
LDFLAGS = -fpic -pthread -lssl -lcrypto -ldl -lcrypt -L$(RHOME)/lib\
 -lR -lRlapack -lRblas

.SUFFIXES: .c

.PHONY: all
all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

.PHONY: install
install:
	@sudo cp $(PROJECT) $(RHOME)/bin/

.PHONY: clean
clean:
	@$(RM) $(PROJECT) *.o *~

standalone.o: Rserv.c
