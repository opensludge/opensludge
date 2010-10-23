/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Common.h - Part of the SLUDGE Dev Kit (GTK+ version)
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

#ifdef __cplusplus
extern "C" {
#endif
const char * getTempDir ();
bool askAQuestion (const char * head, const char * msg);
bool errorBox (const char * head, const char * msg);
#ifdef __cplusplus
}
#endif

char * joinTwoStrings (const char * a, const char * b);
bool fileExists(char * file);
void flipBackslashes(char **string);
void winChangeToProgramDir(const char *programFullPath);
int sh_cmd (char * path, const char * cmd, char * args);
