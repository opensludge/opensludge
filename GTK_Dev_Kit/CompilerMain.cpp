/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * CompilerMain.cpp
 *
 * Copyright (C) 2010 Tobias Hansen <tobias.han@gmx.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <getopt.h>
#include <glib.h>
#include "project.hpp"
#include "compiler.hpp"
#include "interface.h"

//The functions declared in interface.h:

bool askAQuestion (const char * head, const char * msg)
{
	return true;
}

bool errorBox (const char * head, const char * msg)
{
	fprintf(stderr, "%s\n%s\n", head, msg);

	return false;
}

const char * getTempDir ()
{
	return g_get_user_cache_dir();
}

bool fileExists(char * file) {
	FILE * tester;
	bool retval = false;
	tester = fopen (file, "rb");
	if (tester) {
		retval = true;
		fclose (tester);
	}
	return retval;
}

void printCmdlineUsage() {
	fprintf(stdout, "SLUDGE compiler, usage: sludge-compiler <project file>\n");
}

bool parseCmdlineParameters(int argc, char *argv[]) {
	int retval = true;
	while (1)
	{
		static struct option long_options[] =
		{
			{"help",	no_argument,	   0, 'h' },
			{0,0,0,0} /* This is a filler for -1 */
		};
		int option_index = 0;
		char c = getopt_long (argc, argv, "h", long_options, &option_index);
		if (c == -1) break;
			switch (c) {
		case 'h':
		default:
			retval = false;
			break;
		}
	}
	return retval;
}

int main (int argc, char *argv[])
{
	char *fileList[1000];
	int fileListNum;

	if (! parseCmdlineParameters(argc, argv) ) {
		printCmdlineUsage();
		return 0;
	}

	if (argc < 2 || ! fileExists(argv[argc - 1]) ) {
		fprintf(stderr, "Project file not found.\n");
		printCmdlineUsage();
		return -1;
	}


	if (!loadProject (argv[argc - 1], fileList, &fileListNum))
	{
		fprintf(stderr, "Error loading project.\n");
		return -1;
	}
	fprintf(stderr, "Start compiling...\n");
	if (!compileEverything(argv[argc - 1], fileList, &fileListNum, NULL))
	{
		fprintf(stderr, "Error compiling project.\n");
		return -1;
	}

	return 0;
}
