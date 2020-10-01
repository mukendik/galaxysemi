# ---------------------------------------------------------------------------- #
# Makefile for glx
# ---------------------------------------------------------------------------- #
# H=host U=user make send
# H=mac U=gexprod make send
# H=192.168.1.39 U=gexprod make send
# ---------------------------------------------------------------------------- #
PROJECT  = glx
OBJECTS  = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
CXXFLAGS = -O2 -Wall -Wextra -Werror -I/usr/X11R6/include
LDFLAGS  = -L/usr/X11R6/lib -lX11

PROG     = $(PROJECT)

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
