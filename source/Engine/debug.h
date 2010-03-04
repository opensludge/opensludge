#define DEBUGGING		0


#if DEBUGGING
	void debugOut(char * a, ...);
#else
	#define debugOut(a)
#endif
