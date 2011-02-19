#include <string.h>
#include "splitter.hpp"
#include "helpers.h"
#include "messbox.h"
#include "checkused.h"

static bool * flagUsed[CHECKUSED_NUM];

void checkUsedInit (int type, int n)
{
	flagUsed[type] = new bool[n];
	memset (flagUsed[type], 0, sizeof(bool) * n);
}

void warnAboutUnused (int type, stringArray * temp, const char * preamble, stringArray * files) {
	int i = 0;

	if (! flagUsed[type])
		return;

	while (temp)
	{
		if (! flagUsed[type][i])
		{
			char * buff = joinStrings (preamble, temp->string, " not used");
			addCommentWithLine(ERRORTYPE_PROJECTWARNING, buff, files ? files->string : 0, temp->line);
			delete buff;
		}
		i ++;
		temp = temp->next;
		files = files ? files->next : 0;
	}
}

void setUsed (int type, int i)
{
	if (i >= 0 && flagUsed[type])
		flagUsed[type][i] = true;
}
