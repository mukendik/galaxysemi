# ---------------------------------------------------------------------------- #
# Makefile pour testfpu sur linux, cygwin, mingw, et sunos
# ---------------------------------------------------------------------------- #
#
# H=gexprod@dunharrow-prod    make send
# H=gexprod@durthang-prod     make send
# H=gexprod@barad-nimras-prod make send
# H=gexprod@lugburz-prod      make send
# H=gexprod@dol-guldur-prod   make send
# H=gexprod@isengard-prod     make send
#
# mingw32 32 bits :
#  vi /cygdrive/c/Documents\ and\ Settings/gexprod/testfpu.bat
#   cd c:\cygwin\home\gexprod\testfpu
#   set PATH=c:/MinGW/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   mingw32-make
#   testfpu.exe 1
#
# mingw32 64 bits :
#  vi /cygdrive/c/Users/gexprod/testfpu.bat
#   cd c:\cygwin\home\gexprod\testfpu
#   set PATH=c:/mingw64tdm/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   mingw32-make
#   testfpu.exe 1

PROJET = testfpu
CXXFLAGS = -O2 -Wall -Wextra -Werror

ifneq "$(ARCH)" ""
  PROG = $(PROJET).exe
  RM = del
  ifeq "$(ARCH)" "x86"
    CXXFLAGS += -mfpmath=sse -march=pentium4
  endif
else
  PROG = $(PROJET)
  ifeq "$(shell uname -p)" "sparc"
    CXXFLAGS += -DSPARC -mcpu=ultrasparc3
  else
    ifneq "$(shell uname -p)" "x86_64"
      CXXFLAGS += -mfpmath=sse -march=pentium4
    endif
  endif
endif

.SUFFIXES:

.PHONY: all
all: $(PROG)

$(PROG): $(PROJET).o
	$(CXX) $< -o $@

.PHONY: test
test: $(PROG)
	./$(PROG) 1

.PHONY: clean
clean:
	$(RM) $(PROJET).o $(PROG) *~

.PHONY: send
send:
	rsync --checksum -ti $(MAKEFILE_LIST) *.cpp *.h \
	$(H):/home/gexprod/$(PROJET)/

%.o: %.cpp $(MAKEFILE_LIST)
	$(CXX) -c $(CXXFLAGS) $< -o $@
