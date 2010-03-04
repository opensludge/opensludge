# GNU make makefile for OpenSLUDGE by Tobias Hansen
#
# Switch comments to build with either OpenAL
# or BASS as sound library:
#
#--OpenGL/Alure:-------------------------------------
SOUND_LDFLAGS = -lalure
SOUND_OBJ = sound_openal.o
#--BASS:---------------------------------------------
#SOUND_LDFLAGS = -lbass
#SOUND_OBJ = sound_bass.o
#----------------------------------------------------

CC      = /usr/bin/g++
PREFIX  = /usr/local
CFLAGS  = -Wall -g -D_REENTRANT
IFLAGS  = -I `pwd`/../CommonCode
LDFLAGS = -lGL -lGLU -lSDL -lpng $(SOUND_LDFLAGS)

OBJ = $(SOUND_OBJ) backdrop.o bg_effects.o builtin.o cursors.o debug.o fileset.o floor.o fonttext.o freeze.o graphics.o language.o line.o linuxstuff.o loadsave.o main.o memwatch.o moreio.o newfatal.o objtypes.o people.o region.o savedata.o shaders.o sludger.o sprbanks.o sprites.o sprites_aa.o statusba.o stringy.o talk.o thumbnail.o timing.o transition.o variable.o winstuff.o zbuffer.o GLee.o

BIN = sludge_engine

.PHONY: clean install remove

sludge_engine: $(OBJ)
	$(CC) $(CFLAGS) -o sludge_engine $(OBJ) $(IFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)

install: $(BIN) $(OBJ)
	mkdir -p $(PREFIX)/bin
	cp -t $(PREFIX)/bin $(BIN)
	mkdir -p $(PREFIX)/lib/sludge
	cp -t $(PREFIX)/lib/sludge $(OBJ)

remove:
	rm -f $(PREFIX)/bin/sludge_engine
	rm -rf $(PREFIX)/lib/sludge
