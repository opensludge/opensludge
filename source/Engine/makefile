# GNU make makefile for OpenSLUDGE by Tobias Hansen

VERSION = 2.0b
CC      = /usr/bin/g++
PREFIX  = /usr/local
CFLAGS  = -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFLAGS = -lGL -lglee -lSDL -lpng -lbass
# LDFLAGS = -lGL -lglee -lSDL -lpng

OB = BACKDROP.o bg_effects.o builtin.o CURSORS.o debug.o FILESET.o floor.o FONTTEXT.o FREEZE.o Graphics.o Language.o line.o linuxstuff.o LOADSAVE.o main.o memwatch.o MOREIO.o NEWFATAL.o OBJTYPES.o people.o REGION.o savedata.o shaders.o SLUDGER.o SPRBANKS.o SPRITES.o Sprites_AA.o STATUSBA.o STRINGY.o TALK.o thumbnail.o timing.o transition.o VARIABLE.o WINSTUFF.o zbuffer.o

OBJ = $(OB) sound_bass.o
# OBJ = $(OB) sound_nosound.o

BIN = sludge_engine

.PHONY: catch_headers clean install remove

sludge_engine: catch_headers $(OBJ)
	$(CC) $(CFLAGS) -o sludge_engine $(OBJ) $(LDFLAGS)

catch_headers:
	cp -t `pwd` ../CommonCode/functionList.h ../CommonCode/specialSettings.h ../CommonCode/version.h 

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

%.o: %.CPP
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(BIN) $(OBJ) functionList.h specialSettings.h version.h

install: $(BIN) $(OBJ)
	mkdir -p $(PREFIX)/bin
	cp -t $(PREFIX)/bin $(BIN)
	mkdir -p $(PREFIX)/lib/sludge
	cp -t $(PREFIX)/lib/sludge $(OBJ)

remove:
	rm -f $(PREFIX)/bin/sludge_engine
	rm -rf $(PREFIX)/lib/sludge
