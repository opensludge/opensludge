/*
 *  movie.cpp
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2011-02-27.
 *  Copyright 2011 SLUDGE Developers. All rights reserved.
 *
 */

#include "timing.h"
#include "movie.h"

// in main.c
int checkInput();
extern int weAreDoneSoQuit;

// Sludger.cpp
bool handleInput ();

/*
 movieIsPlaying tracks the state of movie playing
 0 = no movie
 1 = movie is played
 2 = paused
*/
int movieIsPlaying = 0;

int playMovie (int fileNumber)
{
	movieIsPlaying = 1;
	
	Init_Special_Timer(24);
	
	while ( movieIsPlaying ) {
		
		checkInput();
		
		if (weAreDoneSoQuit) {
			return 0;
		}
		handleInput ();
		
		
//		sludgeDisplay ();
		Wait_Frame();
	}
	movieIsPlaying = 0;
	Init_Timer();
	return 1;
}

int stopMovie ()
{
	movieIsPlaying = 0;
	return 0;
}

int pauseMovie()
{
	if (movieIsPlaying == 1)
		movieIsPlaying = 2;
	else if (movieIsPlaying == 2)
		movieIsPlaying = 1;
	return movieIsPlaying;
}
