/*
 *  movie.h
 *  Sludge Engine
 *
 *  Created by Rikard Peterson on 2011-02-27.
 *  Copyright 2011 SLUDGE Developers. All rights reserved.
 *
 */


/*
 movieIsPlaying tracks the state of movie playing
 */
enum movieStates {
	nothing = 0,
	playing,
	paused
};
extern movieStates movieIsPlaying;

int playMovie (int fileNumber);
int stopMovie ();
int pauseMovie();
