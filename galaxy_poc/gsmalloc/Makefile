# ---------------------------------------------------------------------------- #
# Makefile pour gsmalloc
# ---------------------------------------------------------------------------- #
#
# H=gexprod@dunharrow-prod    make send
# H=gexprod@durthang-prod     make send
# H=gexprod@barad-nimras-prod make send
# H=gexprod@lugburz-prod      make send
# H=gexprod@dol-guldur-prod   make send
# H=gexprod@isengard-prod     make send
#
# cygwin :
#  [ARCH=1] make
#  GS_MEMORY_TRACER=1 ./gsmalloc
#
# mingw32 32 bits :
#  vi /cygdrive/c/Documents\ and\ Settings/gexprod/gsmalloc.bat
#   cd c:\cygwin\home\gexprod\gsmalloc
#   set PATH=c:/MinGW/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   mingw32-make
#   set GS_MEMORY_TRACER=1
#   gsmalloc.exe
#
# mingw32 64 bits :
#  vi /cygdrive/c/Users/gexprod/gsmalloc.bat
#   cd c:\cygwin\home\gexprod\gsmalloc
#   set PATH=c:/mingw64tdm/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   mingw32-make
#   set GS_MEMORY_TRACER=1
#   gsmalloc.exe

PROJET   = gsmalloc
OBJETS   = $(PROJET).o main.o
CXXFLAGS = -O2 -Wall -Wextra -Werror

ifneq "$(ARCH)" ""
  PROG = $(PROJET).exe
  RM = del
  ifeq "$(ARCH)" "x86"
    CXXFLAGS += -mfpmath=sse -march=pentium4
  endif
else
  PROG = $(PROJET)
  ifneq "$(shell uname -p)" "sparc"
    ifneq "$(shell uname -p)" "x86_64"
      CXXFLAGS += -mfpmath=sse -march=pentium4
    endif
  endif
endif

.SUFFIXES:

.PHONY: all
all: $(PROG)

$(PROG): $(OBJETS)
	$(CXX) $^ -o $@
	strip $(PROG)

.PHONY: test
test: $(PROG)
	./$(PROG) 1

.PHONY: clean
clean:
	$(RM) $(OBJETS) $(PROG) *~

.PHONY: send
send:
	rsync --checksum -ti $(MAKEFILE_LIST) *.cpp *.h \
	$(H):/home/gexprod/$(PROJET)/

%.o: %.cpp $(MAKEFILE_LIST)
	$(CXX) -c $(CXXFLAGS) $< -o $@

gsmalloc.o: gsmalloc.h
main.o: gsmalloc.h
