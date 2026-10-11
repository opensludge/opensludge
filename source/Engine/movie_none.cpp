/*
 * We build this file instead of movie.cpp when SLUDGE_MOVIES is off, which
 * is for builds without libvpx.
 */

#include "debug.h"
#include "movie.h"

movieStates movieIsPlaying = nothing;

int playMovie (int fileNumber) {
	debugOut ("This engine was built without movie support.\n");
	return 0;
}

int stopMovie () {
	return 0;
}

int pauseMovie () {
	return 0;
}

void setMovieViewport () {
}
