#ifndef __APPLE__


#include <stdio.h>
#include "compilerinfo.h"


void (*setInfo)(compilerInfo *);
double progress1Max, progress2Max;

compilerInfo * emptyCompilerInfo()
{
	compilerInfo *info = new compilerInfo;
	info->progress1 = -1.;
	info->progress2 = -1.;
	info->task[0] = 0;
	info->file[0] = 0;
	info->item[0] = 0;
	info->funcs = -1;
	info->objs = -1;
	info->globs = -1;
	info->strings = -1;
	info->resources = -1;
	info->newComments = false;
	info->finished = false;
	info->success = false;

	return info;
}

void setInfoReceiver(void (*infoReceiver)(compilerInfo *))
{
	setInfo = infoReceiver;
}

void clearRect(int i, int whichBox)
{
	if (whichBox == P_TOP) {
		progress1Max = (double)i;
	} else {
		progress2Max = (double)i;
	}
	percRect(0, whichBox);
}

void percRect(unsigned int i, int whichBox)
{
	if (!setInfo) return;

	compilerInfo *info = emptyCompilerInfo();
	if (whichBox == P_TOP) {
		info->progress1 = ((double)i/progress1Max <= 1.) ? (double)i/progress1Max : 1.;
	} else {
		info->progress2 = ((double)i/progress2Max <= 1.) ? (double)i/progress2Max : 1.;
	}
	(*setInfo)(info);
}

void setCompilerText (const compilerStatusText where, const char * theText)
{
	if (!setInfo) return;
	if (!theText) return;

	compilerInfo *info = emptyCompilerInfo();

	switch (where) {
		case COMPILER_TXT_ACTION:
			sprintf(info->task, "%s", theText);
			break;
		case COMPILER_TXT_FILENAME:
			sprintf(info->file, "%s", theText);
			break;
		case COMPILER_TXT_ITEM:
			sprintf(info->item, "%s", theText);
			break;
	}
	(*setInfo)(info);
}

void setCompilerStats(int funcs, int objTypes, int resources, int globals, int strings)
{
	if (!setInfo) return;

	compilerInfo *info = emptyCompilerInfo();
	info->funcs = funcs;
	info->objs = objTypes;
	info->globs = globals;
	info->strings = strings;
	info->resources = resources;
	(*setInfo)(info);
}


void compilerCommentsUpdated()
{
	if (!setInfo) return;

	compilerInfo *info = emptyCompilerInfo();
	info->newComments = true;
	(*setInfo)(info);
}

void setFinished(bool success)
{
	if (!setInfo) return;

	compilerInfo *info = emptyCompilerInfo();
	info->success = success;
	info->finished = true;
	(*setInfo)(info);
}

#endif
