/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Common.cpp - Part of the SLUDGE Dev Kit (GTK+ version)
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "Common.h"
#include "interface.h"

gint message_dialog( GtkMessageType type, const gchar *title, const gchar *message);

bool askAQuestion (const char * head, const char * msg)
{
	gint response;

	response = message_dialog(GTK_MESSAGE_QUESTION, head, msg);

	if (response == GTK_RESPONSE_YES) return true;
		else return false;
}

bool errorBox (const char * head, const char * msg)
{
	message_dialog(GTK_MESSAGE_ERROR, head, msg);

	return false;
}

gint
message_dialog(GtkMessageType type, const gchar *title, const gchar *message)
{
	GtkWidget *dialog;
	gint response;
	GtkButtonsType buttons;

	if (type == GTK_MESSAGE_QUESTION) buttons = GTK_BUTTONS_YES_NO;
	else buttons = GTK_BUTTONS_OK;
        
	dialog = gtk_message_dialog_new (NULL,
						GTK_DIALOG_MODAL,
						type,
						buttons,
						"%s", message);
                        
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);   
        
	return response;
}

void setFileChooserFilters(GtkFileChooser *theFileChooser, gboolean png, gboolean tga)
{
	GtkFileFilter *pngtgafilter, *pngfilter, *tgafilter;

	if (png && tga) {
		pngtgafilter = gtk_file_filter_new();
		gtk_file_filter_set_name(pngtgafilter, "PNG/TGA images");
		gtk_file_filter_add_mime_type(pngtgafilter, "image/png");
		gtk_file_filter_add_mime_type(pngtgafilter, "image/x-tga");
		gtk_file_filter_add_pattern(pngtgafilter, "*.[tT][gG][aA]");
		gtk_file_chooser_add_filter(theFileChooser, pngtgafilter);
	}
	if (png) {
		pngfilter = gtk_file_filter_new();
		gtk_file_filter_set_name(pngfilter, "PNG images");
		gtk_file_filter_add_mime_type(pngfilter, "image/png");
		gtk_file_chooser_add_filter(theFileChooser, pngfilter);
	}
	if (tga) {
		tgafilter = gtk_file_filter_new();
		gtk_file_filter_set_name(tgafilter, "TGA images");
		gtk_file_filter_add_mime_type(tgafilter, "image/x-tga");
		gtk_file_filter_add_pattern(tgafilter, "*.[tT][gG][aA]");
		gtk_file_chooser_add_filter(theFileChooser, tgafilter);
	}
	if (png && tga) {
		gtk_file_chooser_set_filter(theFileChooser, pngtgafilter);
	} else if (png) {
		gtk_file_chooser_set_filter(theFileChooser, pngfilter);
	} else if (tga) {
		gtk_file_chooser_set_filter(theFileChooser, tgafilter);
	}
}

const char * getTempDir ()
{
	return g_get_user_cache_dir();
}

char * joinTwoStrings (const char * a, const char * b) {
	char * nS = new char[strlen (a) + strlen (b) + 1];
	sprintf (nS, "%s%s", a, b);
	return nS;
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

void flipBackslashes(char **string)
{
#ifdef __WIN32
	for (int i = 0; (*string)[i] != 0; i++)
	{
		if ((*string)[i] == '\\')
			(*string)[i] = '/';
	}
#endif
}

void winChangeToProgramDir(const char *programFullPath)
{
#ifdef __WIN32
	char programPath[1000];
	int lastSlash = 0;
	sprintf(programPath, "%s", programFullPath);
	for (int j = 0; programPath[j] != 0; j++) {
		if (programPath[j] == '\\') {
			programPath[j] = '/';
			lastSlash = j;
		}
	}
	programPath[lastSlash + 1] = NULL;
	g_chdir(programPath);
#endif
}

void replaceInvalidCharacters(char *string, int *retval)
{
	const gchar *end;
	if (!g_utf8_validate(string, -1, &end)) {
		for (int i = 0; string[i] != 0; i++) {
			if (string[i] == end[0])
				string[i] = '_';
		}
		replaceInvalidCharacters(string, retval);
		*retval = 0;
	} else {
		return;
	}
}

int sh_cmd (gchar * path, const gchar * cmd, gchar * args)
{
	char     *quoted_args;
	char     cmd_line[256];
	char   **argv;
	int      argp;
	int      rc = 0;

	if (cmd == NULL)
		return FALSE;

	if (cmd[0] == '\0')
		return FALSE;

	if (path != NULL)
		g_chdir (path);

	if (args == NULL) {
		snprintf (cmd_line, sizeof (cmd_line), "%s", cmd);
	} else {
		#ifdef __WIN32
		for (int i = 0; args[i] != 0; i++)
		{
			if (args[i] == '/')
				args[i] = '\\';
		}
		#endif

		quoted_args = g_shell_quote (args);

		snprintf (cmd_line, sizeof (cmd_line), "%s %s", cmd, quoted_args);

		g_free(quoted_args);
	}

	rc = g_shell_parse_argv (cmd_line, &argp, &argv, NULL);
	if (!rc)
	{
		g_strfreev (argv);
		return rc;
	}

	rc = g_spawn_async (path, argv, NULL,
		      G_SPAWN_SEARCH_PATH,
		      NULL, NULL, NULL, NULL);

	g_strfreev (argv);

	return rc;
}
