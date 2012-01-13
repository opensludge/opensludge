#include "movie.h"
#if !defined(HAVE_GLES2)
#include "GLee.h"
#else
#include <GLES2/gl2.h>
#endif

movieStates movieIsPlaying = nothing;

int movieIsEnding = 0;

float movieAspect = 1.6;


GLuint yTextureName = 0;
GLuint uTextureName = 0;
GLuint vTextureName = 0;

int playMovie (int fileNumber) {return 0;};
int stopMovie () {return 0;};
int pauseMovie() {return 0;};
void setMovieViewport() {};
