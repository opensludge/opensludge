# GNU make makefile for OpenSLUDGE by Tobias Hansen

VERSION = 2.0b
CC      = /usr/bin/g++
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\" -fPIC
IFLAGS  = 

OBJ = floormaker.o

.PHONY: clean

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

clean:
	rm -rf $(OBJ)
