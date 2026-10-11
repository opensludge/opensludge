#include <SDL3/SDL.h>

#include "sound.h"

int desiredfps = 300;				//holds desired frames per second
Uint32 starttime,endtime;
Uint32 desired_frame_time;

void Init_Timer(void)
{
	desired_frame_time = 1000/desiredfps;
	starttime = (Uint32) SDL_GetTicks();
}

void Init_Special_Timer(int t)
{
	desired_frame_time = 1000/t;
	starttime = (Uint32) SDL_GetTicks();
}

void Wait_Frame (void (* whileWaiting) (Uint32 millisecondsLeft))
{
	static Uint32 addNextTime = 0;
	Uint32 timetaken;

	updateSounds ();
	
	for (;;) {
		endtime = (Uint32) SDL_GetTicks();
		timetaken = addNextTime + endtime - starttime;
		if (timetaken >= desired_frame_time) break;
		if (whileWaiting) whileWaiting (desired_frame_time - timetaken);
		SDL_Delay(1);
	}

	addNextTime = timetaken - desired_frame_time;
	if (addNextTime > desired_frame_time) addNextTime = desired_frame_time;
	
	starttime = endtime;
}

