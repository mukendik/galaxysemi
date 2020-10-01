# ---------------------------------------------------------------------------- #
# Makefile for qscriptengine
# ---------------------------------------------------------------------------- #
# Examples
#
# echo "QTDIR = /data/qt/5.2.1" >config.mk
#
# echo "QTDIR = /home/gexprod/qt/5.2.1" >config.mk
#
# echo "QTDIR = c:/cygwin/home/gexprod/qt/5.2.1" >config.mk
# echo "QTWIN = /home/gexprod/qt/5.2.1" >>config.mk
#
# echo "QTVER = 5" >>config.mk
#
# make test
# make test E="print('coucou')"
#
# make test WIN=1
# make test E="print('coucou')"
#
# H=host U=user make send
# H=mac U=gexprod make send
#
# make clean
# ---------------------------------------------------------------------------- #

include config.mk
PROJECT  = qscriptengine
OBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
QT       = Core Script
QTINC    = $(addprefix -I$(QTDIR)/include/Qt,$(QT)) -I$(QTDIR)/include
QTLIB    = $(addprefix -lQt$(QTVER),$(QT))
CXXFLAGS = -O1 -D_FORTIFY_SOURCE=2 -Wall -Wextra -Werror $(QTINC)
LDFLAGS  = -L$(QTDIR)/lib $(QTLIB)
ifneq "$(WIN)" ""
  RM = del
  PATH := $(QTWIN)/lib:/cygdrive/c/icu/dist/lib:$(PATH)
else
  CXXFLAGS += -fPIC
endif

PROG = $(PROJECT)

.SUFFIXES: .cpp

.PHONY: all
all: $(PROG)

$(PROG): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

.PHONY: test
test: $(PROG)
	@LD_LIBRARY_PATH=$(QTDIR)/lib \
	DYLD_LIBRARY_PATH=$(QTDIR)/lib \
	$(TRACER) ./$(PROG) "$(E)"

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROG) *~

.PHONY: send
send:
	@rsync --checksum -t -i Makefile *.cpp \
	$(U)@$(H):/home/gexprod/$(PROJECT)/
