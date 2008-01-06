#include <windows.h>
#include "winversion.h"

int desiredfps = 300;				//holds desired frames per second
LARGE_INTEGER systemfrequency={0},starttime,endtime;
LONGLONG desired_frame_time;

void Init_Timer(void)
{
	timeBeginPeriod(1);

	QueryPerformanceFrequency(&systemfrequency);
	desired_frame_time = systemfrequency.QuadPart/desiredfps;
	QueryPerformanceCounter(&starttime);
}

void Wait_Frame (void)
{
	static LONGLONG addNextTime = 0;
	LONGLONG timetaken;
	
	for (;;) {
		QueryPerformanceCounter (&endtime);
		timetaken = addNextTime + endtime.QuadPart - starttime.QuadPart;
		if (timetaken >= desired_frame_time) break;
		Sleep(1);
	}

	addNextTime = timetaken - desired_frame_time;
	if (addNextTime > desired_frame_time) addNextTime = desired_frame_time;
	
	starttime.QuadPart = endtime.QuadPart;
}
