# ---------------------------------------------------------------------------- #
# Makefile for memsize
# ---------------------------------------------------------------------------- #
# H=host U=user make send
# H=mac U=gexprod make send
# H=192.168.1.55 U=gexprod make send
#
# mingw32 :
#  vi "/cygdrive/c/Documents and Settings/gexprod/memsize.bat"
#   @echo off
#   cd c:/cygwin/home/gexprod/memsize
#   set PATH=c:/TDM-GCC-64-4.8.1/bin;c:/mingw32/bin;%PATH%
#   set ARCH=%PROCESSOR_ARCHITECTURE%
#   mingw32-make test
#   cd %homepath%
# ---------------------------------------------------------------------------- #
PROJECT  = memsize
OBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CXXFLAGS = -O2 -Wall -Wextra -Werror

ifneq "$(ARCH)" ""
 PROG    = $(PROJECT).exe
 LDFLAGS = -lpsapi
 RM      = del
else
 PROG    = $(PROJECT)
endif

.SUFFIXES:

.PHONY: all
all: $(PROG)

$(PROG): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@
	strip $(PROG)

.PHONY: test
test: $(PROG)
	./$(PROG)

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(PROG) *~

.PHONY: send
send:
	@rsync --checksum -t -i Makefile *.cpp \
	$(U)@$(H):/home/gexprod/$(PROJECT)/

%.o: %.cpp $(MAKEFILE_LIST)
	$(CXX) -c $(CXXFLAGS) $< -o $@
