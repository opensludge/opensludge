#ifndef __COMPILERINFO_H__
#define __COMPILERINFO_H__

typedef struct
{
	double progress1;
	double progress2;
	char task[1000];
	char file[1000];
	char item[1000];
	int funcs;
	int objs;
	int globs;
	int strings;
	int resources;
	bool newComments;
	bool finished;
	bool success;
} compilerInfo;

enum whichPerc {
	P_TOP,
	P_BOTTOM
};

enum  compilerStatusText{
	COMPILER_TXT_ACTION,
	COMPILER_TXT_FILENAME,
	COMPILER_TXT_ITEM
};

void setInfoReceiver(void (*infoReceiver)(compilerInfo *));
void clearRect(int i, int whichBox);
void percRect(unsigned int i, int whichBox);
void setCompilerText(const int where, const char * tx);
void setCompilerStats(int funcs, int objTypes, int resources, int globals, int strings);
void compilerCommentsUpdated();
void setFinished(bool success);

#endif
