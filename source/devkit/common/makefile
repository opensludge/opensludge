# GNU make makefile for OpenSLUDGE by Tobias Hansen

VERSION = 2.0b
CC      = /usr/bin/g++
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\" -fPIC
IFLAGS  = -isystem`gnustep-config --variable=GNUSTEP_SYSTEM_HEADERS`

OBJ = macstuff.o messbox.o moreio.o splitter.o tga.o

.PHONY: clean

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

%.o: %.mm
	$(CC) $(CFLAGS) -xobjective-c++ -fconstant-string-class=NSConstantString -c $< $(IFLAGS)

clean:
	rm -rf $(OBJ)
