bin_PROGRAMS = sludge-engine

sludge_engine_SOURCES = \
	backdrop.cpp bg_effects.cpp builtin.cpp cursors.cpp debug.cpp \
	fileset.cpp floor.cpp fonttext.cpp freeze.cpp graphics.cpp helpers.cpp \
	language.cpp line.cpp linuxstuff.cpp loadsave.cpp main.cpp \
	memwatch.cpp moreio.cpp movie.cpp newfatal.cpp objtypes.cpp people.cpp \
	region.cpp savedata.cpp shaders.cpp sludger.cpp sprbanks.cpp \
	sprites.cpp statusba.cpp stringy.cpp talk.cpp \
	thumbnail.cpp timing.cpp transition.cpp variable.cpp \
	zbuffer.cpp sound_openal.cpp \
	allfiles.h backdrop.h bg_effects.h builtin.h colours.h csludge.h \
	cursors.h debug.h errors.h fileset.h floor.h fonttext.h freeze.h \
	graphics.h helpers.h language.h line.h loadsave.h memwatch.h moreio.h \
	movie.h newfatal.h objtypes.h people.h region.h savedata.h shaders.h \
	sludger.h sound.h sprbanks.h sprites.h statusba.h \
	stringy.h talk.h thumbnail.h timing.h transition.h variable.h \
	platform-dependent.h linuxstuff.h zbuffer.h \
	../CommonCode/utf8.cpp ../CommonCode/utf8.h \
	../CommonCode/functionlist.h \
	../CommonCode/specialsettings.h \
	../CommonCode/version.h \
	libvorbis/vorbis_misc.h libvorbis/vorbis_os.h \
	libwebm/mkvparser.cpp libwebm/mkvparser.hpp \
	libwebm/mkvreader.cpp libwebm/mkvreader.hpp 

if COND_GLES2
  sludge_engine_SOURCES += eglport/eglport.cpp eglport/eglport.h
endif

dist_pkgdata_DATA = \
	fixScaleSprite.frag fixScaleSprite.vert \
	scale.frag scale_noaa.frag scale.vert \
	texture.frag texture.vert \
	color.frag color.vert \
	yuv.frag yuv.vert

AM_CPPFLAGS = -std=c++0x -I$(top_srcdir)/source/CommonCode -I$(top_srcdir)/source/Engine/libwebm @ENGINE_CFLAGS@ @LIBPNG_CFLAGS@ -DDATADIR='"$(pkgdatadir)/"'

LDADD = @ENGINE_LIBS@ @LIBPNG_LIBS@

if COND_GLES2
  AM_CPPFLAGS += -DHAVE_GLES2 -DUSE_EGL_SDL -DUSE_GLES2
else
  AM_CPPFLAGS += @GL_CFLAGS@ @GLEW_CFLAGS@
  LDADD += @GL_LIBS@ @GLEW_LIBS@
endif

if COND_PANDORA
  AM_CPPFLAGS += -DPANDORA
  LDADD += -liconv
endif
if COND_RPI
  AM_CPPFLAGS += -DRPI
endif
if COND_ODROID
  AM_CPPFLAGS += -DODROID
endif


EXTRA_DIST = \
	libvorbis/COPYING \
	libwebm/AUTHORS.TXT \
	libwebm/LICENSE.TXT \
	libwebm/PATENTS.TXT
