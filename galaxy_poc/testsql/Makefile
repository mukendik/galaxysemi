# ---------------------------------------------------------------------------- #
# Makefile for testsql
# ---------------------------------------------------------------------------- #
# Examples
#
# vi config.mk
#  QTDIR = /data/qt/5.4.2
#  QTVER = 5
# export MYSQL_PWD
# export ORACLE_HOME
# export ORACLE_PWD
# make test H=host U=user P=pass
# make test H=host U=user P=pass S=sid
# make test H=192.168.1.143 U=root P=$MYSQL_PWD
# make test H=SATURNE U=root P=$MYSQL_PWD
# make test H=SATURNE U="SYS\ AS\ SYSDBA" P=$ORACLE_PWD S=GALAXY
#
# H=host U=user make send
# H=mac U=gexprod make send
#
# make clean
# ---------------------------------------------------------------------------- #
# Cygwin
# ---------------------------------------------------------------------------- #
# vi config.mk
#  QTDIR = c:/cygwin/home/gexprod/qt/Qt-5.4.2
#  QTVER = 5
#  MINGWDIR = /cygdrive/c/mingw64
# cp -a ~/prod/master-gex/libmariadb.dll .
# cp -a ~/prod/master-gex/sqldrivers .
# export MYSQL_PWD
# make test TRACER="gdb --args" H=192.168.1.143 U=root P=$MYSQL_PWD
# ---------------------------------------------------------------------------- #
include config.mk
PROJECT  = testsql
OBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
QT       = Core Sql
QTINC    = $(addprefix -I$(QTDIR)/include/Qt,$(QT)) -I$(QTDIR)/include
QTLIB    = $(addprefix -lQt$(QTVER),$(QT))
CXXFLAGS = -O2 -Wall -Wextra -Werror $(QTINC) -DQT_DEBUG
LDFLAGS  = -L$(QTDIR)/lib $(QTLIB)
ifneq "$(MINGWDIR)" ""
CXXFLAGS += -posix
PROG = $(PROJECT).exe
PATH := $(MINGWDIR)/bin:/usr/bin
else
CXXFLAGS += -fPIC
PROG = $(PROJECT)
endif

.SUFFIXES: .cpp

.PHONY: all
all: $(PROG)

$(PROG): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(PROG)
ifneq "$(MINGWDIR)" ""
	@PATH=/cygdrive/c/icu/dist/lib:$(PATH) \
	PATH=$(shell cygpath -u $(QTDIR))/bin:$(PATH) \
	QT_DEBUG_PLUGINS=1 \
	QT_PLUGIN_PATH=$(shell cygpath -m $(PWD)) \
	$(TRACER) ./$(PROG) $(H) $(U) $(P) $(S)
else
	@QT_DEBUG_PLUGINS=1 \
	LD_LIBRARY_PATH=$(ORACLE_HOME)/lib:$(QTDIR)/lib \
	DYLD_LIBRARY_PATH=$(ORACLE_HOME):$(QTDIR)/lib \
	$(TRACER) ./$(PROG) $(H) $(U) $(P) $(S)
endif

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROG) *~

.PHONY: send
send:
	@rsync --checksum -t -i Makefile *.cpp \
	$(U)@$(H):/home/gexprod/$(PROJECT)/

$(OBJECTS): $(MAKEFILE_LIST)
