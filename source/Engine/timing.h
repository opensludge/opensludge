#include <SDL3/SDL_stdinc.h>



void Init_Timer(void);
void Init_Special_Timer(int t);
void Get_Start_Time(void);
void Get_End_Time(void);
// Wait until it is time for the next frame. While waiting, call whileWaiting
// regularly, when it is not NULL.
void Wait_Frame (void (* whileWaiting) (Uint32 millisecondsLeft) = NULL);
