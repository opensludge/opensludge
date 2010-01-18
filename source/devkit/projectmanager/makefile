# GNU make makefile for OpenSLUDGE by Tobias Hansen

VERSION = 2.0b
CC      = /usr/bin/g++
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\" -fPIC
IFLAGS  = -I../../CommonCode -I../common

OBJ = project.o settings.o

.PHONY: clean

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

clean:
	rm -rf $(OBJ)
