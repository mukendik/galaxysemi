# ---------------------------------------------------------------------------- #
# Makefile for rserve client
# ---------------------------------------------------------------------------- #
# Install server :
#
# cd /data
# ls -l Rserve_1.8-0.tar.gz
# sudo /usr/bin/R CMD INSTALL Rserve_1.8-0.tar.gz
# ---------------------------------------------------------------------------- #
# Install client :
#
# tar xzf Rserve_1.8-0.tar.gz
# patch -p0 -i Rserve_1.8-0.patch
# cd Rserve/src/client/cxx
# ./configure
# make debug
# ---------------------------------------------------------------------------- #
# Run :
#
# make test-server
# make test-client S=1410.csv
# ---------------------------------------------------------------------------- #
# Debug rserve server :
# 
# vi RS-debug.mk
# ---------------------------------------------------------------------------- #
PROJECT  = rserve
RPREFIX  = /usr
RSERVE   = /data/Rserve/clients/cxx
CXXFLAGS = -g -O2 -Wall -Wextra -I$(RSERVE)
LDFLAGS  = -lcrypt

.SUFFIXES: .cpp

.PHONY: all
all: $(PROJECT)

$(PROJECT): main.o
	$(CXX) $< $(RSERVE)/Rconnection.o $(CXXFLAGS) $(LDFLAGS) -o $@

.PHONY: test-server
test-server:
	@$(RPREFIX)/bin/R CMD Rserve --RS-socket /tmp/RS-socket

.PHONY: test-client
test-client: $(PROJECT)
	@valgrind --leak-check=full --show-leak-kinds=all --num-callers=24 \
	./$(PROJECT) $(S) | \
	sed '/-nan/D' | \
	gnuplot -e "\
	plot '-' pt 7 ps 0.5 t 'Normal theoretical quantiles'; \
	pause mouse key;" \
	2>/dev/null

.PHONY: clean
clean:
	$(RM) $(PROJECT) *.o *~
