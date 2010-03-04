#ifdef __linux__
#include <SDL/SDL.h>
#else
#include "SDL.h"
#endif

int desiredfps = 300;				//holds desired frames per second
Uint32 starttime,endtime;
Uint32 desired_frame_time;

void Init_Timer(void)
{
	desired_frame_time = 1000/desiredfps;
	starttime = SDL_GetTicks();
}

void Wait_Frame (void)
{
	static Uint32 addNextTime = 0;
	Uint32 timetaken;
	
	for (;;) {
		endtime = SDL_GetTicks();
		timetaken = addNextTime + endtime - starttime;
		if (timetaken >= desired_frame_time) break;
		SDL_Delay(1);
	}

	addNextTime = timetaken - desired_frame_time;
	if (addNextTime > desired_frame_time) addNextTime = desired_frame_time;
	
	starttime = endtime;
}
