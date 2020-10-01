# ---------------------------------------------------------------------------- #
# Makefile for lssqldrivers
# ---------------------------------------------------------------------------- #
# Examples
#
# echo "QTDIR = /data/qt/5.2.1" >config.mk
# echo "QTDIR = /home/gexprod/qt/5.2.1" >config.mk
# echo "QTDIR = c:/cygwin/home/gexprod/qt/5.2.1" >config.mk
#
# echo "QTVER = 5" >>config.mk
#
# export ORACLE_HOME
# export MYSQL_HOME=/usr/share/mysql-connector-c-6.0.2-osx10.5-x86-64bit
# make test
#
# TRACER=strace make test 2>/tmp/strace.out
# TRACER=dtruss make test 2>/tmp/dtruss.out
#
# H=host U=user make send
# H=mac U=gexprod make send
#
# make clean
#
# ---------------------------------------------------------------------------- #
# mingw32:
#
# vi "/cygdrive/c/Users/gexprod/lssqldrivers.bat"
# @echo off
# cd c:/cygwin/home/gexprod/prod/gex-prod-draft/galaxy_poc/lssqldrivers
# set PATH=c:/cygwin/home/gexprod/tmp;%PATH%
# set PATH=c:/cygwin/home/gexprod/qt/5.2.1/lib;c:/icu/dist/lib;%PATH%
# set PATH=c:/TDM-GCC-64-4.8.1/bin;c:/mingw32/bin;%PATH%
# set ARCH=win
# set QT_DEBUG_PLUGINS=1
# set QT_PLUGIN_PATH=c:/cygwin/home/gexprod/tmp
# mingw32-make test
# cd %homepath%
#
# ---------------------------------------------------------------------------- #
# Problems
#
#  - dtruss ==> Library not loaded
#  - if mysql or oci plugin "is not a valid Qt plugin" :
#    rm ~/Library/Preferences/com.trolltech.plist
#
# ---------------------------------------------------------------------------- #

include config.mk
PROJECT  = lssqldrivers
OBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
QT       = Core Sql
QTINC    = $(addprefix -I$(QTDIR)/include/Qt,$(QT)) -I$(QTDIR)/include
QTLIB    = $(addprefix -lQt$(QTVER),$(QT))
CXXFLAGS = -O2 -Wall -Wextra -Werror $(QTINC) -DQT_DEBUG
LDFLAGS  = -L$(QTDIR)/lib $(QTLIB)
ifneq "$(ARCH)" ""
CXXFLAGS += -posix
PROG = $(PROJECT).exe
else
CXXFLAGS += -fPIC
LDFLAGS  +=  -ldl
PROG = $(PROJECT)
endif

.SUFFIXES: .cpp

.PHONY: all
all: $(PROG)

$(PROG): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(PROG)
ifneq "$(ARCH)" ""
	./$(PROG)
else
	@QT_DEBUG_PLUGINS=1 \
	LD_LIBRARY_PATH=$(ORACLE_HOME)/lib:$(QTDIR)/lib \
	DYLD_LIBRARY_PATH=$(ORACLE_HOME):$(MYSQL_HOME)/lib:$(QTDIR)/lib \
	$(TRACER) ./$(PROG)
endif

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROG) *~

.PHONY: send
send:
	@rsync --checksum -t -i Makefile *.cpp \
	$(U)@$(H):/home/gexprod/$(PROJECT)/

$(OBJECTS): $(MAKEFILE_LIST)
