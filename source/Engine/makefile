# GNU make makefile for OpenSLUDGE by Tobias Hansen
#
# Switch comments to build with either OpenAL
# or BASS as sound library:
#
#--OpenGL/Alure:-------------------------------------
#SOUND_LDFLAGS = -lalure -ldumb
#SOUND_OBJ = sound_openal.o
#--BASS:---------------------------------------------
SOUND_LDFLAGS = -lbass
SOUND_OBJ = sound_bass.o
#----------------------------------------------------

CC      = /usr/bin/g++
PREFIX  = /usr/local
CFLAGS  = -Wall -g -D_REENTRANT
IFLAGS  = -I `pwd`/../CommonCode
LDFLAGS = -lGL -lGLU -lSDL -lpng $(SOUND_LDFLAGS)

OBJ = $(SOUND_OBJ) BACKDROP.o bg_effects.o builtin.o CURSORS.o debug.o FILESET.o floor.o FONTTEXT.o FREEZE.o Graphics.o Language.o line.o linuxstuff.o LOADSAVE.o main.o memwatch.o MOREIO.o NEWFATAL.o OBJTYPES.o people.o REGION.o savedata.o shaders.o SLUDGER.o SPRBANKS.o SPRITES.o Sprites_AA.o STATUSBA.o STRINGY.o TALK.o thumbnail.o timing.o transition.o VARIABLE.o WINSTUFF.o zbuffer.o GLee.o



BIN = sludge_engine

.PHONY: clean install remove

sludge_engine: $(OBJ)
	$(CC) $(CFLAGS) -o sludge_engine $(OBJ) $(IFLAGS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

%.o: %.CPP
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(IFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)

install: $(BIN) $(OBJ) sludge_starter
	mkdir -p $(PREFIX)/bin
	cp -t $(PREFIX)/bin $(BIN) sludge_starter
	mkdir -p $(PREFIX)/lib/sludge
	cp -t $(PREFIX)/lib/sludge $(OBJ)

remove:
	rm -f $(PREFIX)/bin/sludge_engine $(PREFIX)/bin/sludge_starter
	rm -rf $(PREFIX)/lib/sludge
