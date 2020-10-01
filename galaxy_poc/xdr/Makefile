# ---------------------------------------------------------------------------- #
# Makefile for xdr
# ---------------------------------------------------------------------------- #
# linux :
#  curl -O https://bsd-xdr.googlecode.com/files/bsd-xdr-1.0.0.tar.gz
#  tar xzf bsd-xdr-1.0.0.tar.gz
#  make lib
#  bsd-xdr-1.0.0/linux/xdrmem_test_static -v -v -v
#  bsd-xdr-1.0.0/linux/xdrsizeof_test_static -v -v -v
#  bsd-xdr-1.0.0/linux/xdrstdio_test_static -v -v -v
#  make test-server
#  make test-client H=127.0.0.1
#  make test-client H=192.168.1.55
#
# mingw32 :
#  make send H=192.168.1.55 U=gexprod
#  scp bsd-xdr-1.0.0.tar.gz gexprod@fornost-prod:/home/gexprod/xdr/
#  cd /home/gexprod/xdr
#  tar xzf bsd-xdr-1.0.0.tar.gz
#  sed -i 's/ -Werror//' bsd-xdr-1.0.0/Makefile
#  vi "/cygdrive/c/Documents and Settings/gexprod/xdr.bat"
#
#   @echo off
#   set PATH=c:/TDM-GCC-64-4.8.1/bin;c:/mingw32/bin;c:/MinGW/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   set pocdir=cygwin/home/gexprod/xdr
#   set xdrdir=%pocdir%/bsd-xdr-1.0.0
#   if "%1" == "lib" goto :lib
#
#   cd c:\%pocdir%
#   mingw32-make %1 H=%2
#   goto :eof
#
#   :lib
#   set PATH=%PATH%;c:/cygwin/bin
#   cd c:\%xdrdir%
#   mingw32-make top_srcdir=c:/%xdrdir% uname=MINGW
#
#   :eof
#   cd %homepath%
#
#  xdr lib
#  xdr test-server
#  xdr test-client 127.0.0.1
#  xdr test-client 192.168.1.??
#
# end :
#  make kill-server H=192.168.1.55
#  make kill-server H=192.168.1.??
# ---------------------------------------------------------------------------- #
PROJECT = xdr
CFLAGS  = -O2 -Wall -Wextra -Werror -Ibsd-xdr-1.0.0 -DNDEBUG

ifneq "$(ARCH)" ""
  CC = gcc
  XDRDIR = c:/cygwin/home/gexprod/xdr/bsd-xdr-1.0.0/mingw
  LDFLAGS = -L$(XDRDIR) -Wl,-Bstatic -lxdr -Wl,-Bdynamic -lws2_32
  RM = del
  CFLAGS += -posix
else
  EXT = .unix
endif

.SUFFIXES: .c

.PHONY: all
all: server client

.PHONY: lib
lib:
	cd bsd-xdr-1.0.0 && MAKELEVEL=0 $(MAKE) -f Makefile$(EXT)

.PHONY: test-server
test-server: server
	@./$< 12345

.PHONY: test-client
test-client: client
	@./$< $(H) 12345

.PHONY: test-bench
test-bench: bench
	@./$< $(H)

.PHONY: kill-server
kill-server:
	curl $(H):12345/kill

.PHONY: clean
clean:
	$(RM) server client bench *.o *~

.PHONY: send
send:
	@rsync --checksum -t -i Makefile *.c *.h \
	$(U)@$(H):/home/gexprod/$(PROJECT)/

server.o: common.h $(MAKEFILE_LIST)
client.o: common.h $(MAKEFILE_LIST)
bench.o: common.h $(MAKEFILE_LIST)

%: %.o
	$(CC) $^ $(LDFLAGS) -o $@
