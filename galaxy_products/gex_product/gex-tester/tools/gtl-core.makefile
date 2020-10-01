#
# Makefile for GTL-CORE library.
# Copyright 2005 by Galaxy
#

# TARGETS
# sunos4				: generate gtl-core library for Teradyne tester on SunOS
# sunos5				: generate gtl-core library for Teradyne tester on Solaris

##################
# Compiler Flags #
##################
CC = cc
sunos4:=CFLAGS=-I./
sunos5:=CFLAGS=-I./

SOURCES.h = utils.h gneterror.h gnetbuffer.h gnetmessage.h gtc_netmessage.h gtc_constants.h gtl_constants.h gtl_error.h gtl_main.h gtl_message.h gtl_server.h gtl_socket.h gtl_testlist.h
SOURCES.c = utils.c gneterror.c gnetbuffer.c gnetmessage.c gtc_netmessage.c gtl_error.c gtl_main.c gtl_message.c gtl_server.c gtl_socket.c gtl_testlist.c

# Derived parameters.

SOURCES = \
	$(SOURCES.c) \
	$(SOURCES.h)

OBJECTS = \
	$(SOURCES.c:%.c=%.o)

# Targets.

sunos4:	$(SOURCES) $(OBJECTS)
	@-$(RM) -rf library.a
	ar rv library.a ${OBJECTS}
	ranlib library.a

sunos5:	$(SOURCES) $(OBJECTS)
	@-$(RM) -rf library.a
	ar rv library.a ${OBJECTS}
	ranlib library.a

clean:
	@-$(RM) -rf $(OBJECTS) *.a *.*% *% *.bak core

delete:
	@-$(RM) -rf $(SOURCES) $(OBJECTS) *.a *.*% *% *.bak core

