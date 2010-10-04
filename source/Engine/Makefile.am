bin_PROGRAMS = sludge-engine

sludge_engine_SOURCES = \
	backdrop.cpp bg_effects.cpp builtin.cpp cursors.cpp debug.cpp \
	fileset.cpp floor.cpp fonttext.cpp freeze.cpp graphics.cpp \
	language.cpp line.cpp linuxstuff.cpp loadsave.cpp main.cpp \
	memwatch.cpp moreio.cpp newfatal.cpp objtypes.cpp people.cpp \
	region.cpp savedata.cpp shaders.cpp sludger.cpp sprbanks.cpp \
	sprites.cpp statusba.cpp stringy.cpp talk.cpp \
	thumbnail.cpp timing.cpp transition.cpp variable.cpp \
	zbuffer.cpp sound_openal.cpp \
	allfiles.h backdrop.h bg_effects.h builtin.h colours.h csludge.h \
	cursors.h debug.h errors.h fileset.h floor.h fonttext.h freeze.h \
	graphics.h language.h line.h loadsave.h memwatch.h moreio.h \
	newfatal.h objtypes.h people.h region.h savedata.h shaders.h \
	sludger.h sound.h sprbanks.h sprites.h statusba.h \
	stringy.h talk.h thumbnail.h timing.h transition.h variable.h \
	platform-dependent.h linuxstuff.h zbuffer.h \
	../CommonCode/functionlist.h \
	../CommonCode/specialsettings.h \
	../CommonCode/version.h

dist_pkgdata_DATA = \
	fixScaleSprite.frag fixScaleSprite.vert \
	scale.frag scale.vert

AM_CPPFLAGS = -I$(top_srcdir)/source/CommonCode @LIBPNG_CFLAGS@ -DDATADIR='"$(pkgdatadir)/"'

LDADD = @LIBPNG_LIBS@
