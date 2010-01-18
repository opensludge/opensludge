# GNU make makefile for OpenSLUDGE by Tobias Hansen

VERSION = 2.0b
CC      = /usr/bin/g++
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\" -fPIC
IFLAGS  = -I../../CommonCode -I../common -I../projectmanager

OBJ = allknown.o checkused.o compiler.o dumpfiles.o floor.o hsi.o linker.o objtype.o percbar.o preproc.o realproc.o sludge_functions.o tokens.o translation.o

.PHONY: clean

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

clean:
	rm -rf $(OBJ)
