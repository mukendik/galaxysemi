#
# Makefile for GTL-TESTER library.
# Copyright 2005 by Galaxy
#

# TARGETS
# sunos4				: generate gtl-tester library for Teradyne tester on SunOS
# sunos5				: generate gtl-tester library for Teradyne tester on Solaris

##################
# Compiler Flags #
##################
CC = cc
sunos4:=CFLAGS=-I./
sunos5:=CFLAGS=-I./

SOURCES.h = utils.h gtl_tester.h
SOURCES.c = utils.c gtl_tester_image.c

# Derived parameters.

SOURCES = \
	$(SOURCES.c) \
	$(SOURCES.h)

OBJECTS = \
	$(SOURCES.c:%.c=%.o)

# Targets.

sunos4:	
	@-$(RM) -rf library.a
	itlc gtl_tester_image.c -incdir . -ofile
	ar rv library.a gtl_tester_image.o
	ranlib library.a

sunos5:	
	@-$(RM) -rf library.a
	itlc gtl_tester_image.c -incdir . -ofile
	ar rv library.a gtl_tester_image.o
	ranlib library.a

clean:
	@-$(RM) -rf $(OBJECTS) *.a *.*% *% *.bak core

delete:
	@-$(RM) -rf $(SOURCES) $(OBJECTS) *.a *.*% *% *.bak core

