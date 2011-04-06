/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeApplication.cpp - Part of the SLUDGE Dev Kit (GTK+ version)
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
#include <glib/gstdio.h>

#ifdef __WIN32
#include <direct.h>
#endif

#include "SludgeApplication.h"
#include "Common.h"

SludgeApplication::SludgeApplication(const char * gladeFileName, const char * iconName, const char * configFile)
{
	configfile = configFile;
	GError *err = NULL;

	initSuccess = TRUE;

	char buf[1000];
	GdkPixbuf *pixbuf16, *pixbuf32, *pixbuf128, *pixbuf256;
	GList *list = NULL;

	sprintf(buf, "%s%s_16x16x32.png", DATADIR, iconName);
	pixbuf16 = gdk_pixbuf_new_from_file (buf, &err);

	if (err == NULL) {
		sprintf(buf, "%s%s_32x32x32.png", DATADIR, iconName);
		pixbuf32 = gdk_pixbuf_new_from_file (buf, &err);
	}
	if (err == NULL) {
		sprintf(buf, "%s%s_128x128x32.png", DATADIR, iconName);
		pixbuf128 = gdk_pixbuf_new_from_file (buf, &err);
	}
	if (err == NULL) {
		sprintf(buf, "%s%s_256x256x32.png", DATADIR, iconName);
		pixbuf256 = gdk_pixbuf_new_from_file (buf, &err);
	}

	if (err != NULL)
	{
		fprintf (stderr, "Unable to open icon file: %s\n", err->message);
		g_error_free (err);
	} else {
		list = g_list_append (list, pixbuf16);
		list = g_list_append (list, pixbuf32);
		list = g_list_append (list, pixbuf128);
		list = g_list_append (list, pixbuf256);
		gtk_window_set_default_icon_list(list);
	}

	/*
	 * Load the GTK interface.
	 */
	theXml = gtk_builder_new ();

	if (!gtk_builder_add_from_file (theXml, gladeFileName, NULL))
	{
		g_critical ("Failed to load the GTK file.\n");
		errorBox("Error!", joinTwoStrings("Failed to load resource file:\n", gladeFileName));
		initSuccess = FALSE;
		return;
	}

	/*
	 * Get the top-level window reference from loaded Glade file.
	 */
	theWindow = GTK_WIDGET (gtk_builder_get_object (theXml, "window1"));

	if (theWindow == NULL)
	{
		g_critical ("Failed to get the window from the builder.\n");
		initSuccess = FALSE;
		return;
	}

	// Set unassigned widgets to get handled automatically by the window manager.
	gtk_container_set_reallocate_redraws (GTK_CONTAINER (theWindow), TRUE);

	char folderFile[300];
	sprintf(folderFile, "%s/sludge-devkit/%s", g_get_user_config_dir(), configfile);
	FILE * fp = fopen (folderFile, "r");
	if (fp) {
		char readChar = ' ';
		for (int i = 0; i < 300; i++) {
			readChar = fgetc(fp);
			if (readChar != '\n') {
				currentFolder[i] = readChar;
			} else {
				currentFolder[i] = 0;
				break;
			}
		}
		fclose (fp);
	} else {
		sprintf (currentFolder, "%s", g_get_home_dir());
	}
	fileChanged = FALSE;

	currentFilename[0] = 0;
	currentShortname[0] = 0;
}

SludgeApplication::~SludgeApplication()
{
	if (currentFolder[0] != 0)
	{
		if (g_chdir(g_get_user_config_dir())) return;
#ifdef __WIN32
		_mkdir("sludge-devkit");
#else
		g_mkdir("sludge-devkit", 0000777);
#endif
		if (g_chdir("sludge-devkit")) return;
		FILE * fp = fopen (configfile, "w");

		if (fp) {
			fprintf (fp, "%s\n", currentFolder);
			fclose (fp);
		}
	}
}

void SludgeApplication::saveToFile()
{
	GtkWidget *dialog;
	GtkFileFilter *filter;

	dialog = gtk_file_chooser_dialog_new ("Save File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, getFilterName());
	gtk_file_filter_add_pattern(filter, getFilterPattern());
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);

	if (currentShortname[0] != 0)
	{
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), currentShortname);
	}
	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		flipBackslashes(&filename);

		if (!saveFile (filename))
		{
			errorBox ("Error", "Saving file failed.");
		}
		else
		{
			setFilename(filename);
			setFolderFromFilename(filename);
		}

		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

gboolean SludgeApplication::reallyClose()
{
	if (fileChanged) {
		GtkWidget *dialog;
		gint response;
		dialog = gtk_message_dialog_new (NULL,
										GTK_DIALOG_MODAL,
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_NONE,
										"Save the changes to '%s' before closing?", currentShortname);

		gtk_dialog_add_buttons(GTK_DIALOG(dialog), 
								"Don't Save", GTK_RESPONSE_REJECT,
								"Cancel", GTK_RESPONSE_CANCEL,
								"Save", GTK_RESPONSE_ACCEPT,
								NULL);
		                    
		gtk_window_set_title(GTK_WINDOW(dialog), "There are unsaved changes.");
		    
		response = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		if (response == GTK_RESPONSE_CANCEL || response == GTK_RESPONSE_NONE) {
			return FALSE;
		}
		if (response == GTK_RESPONSE_ACCEPT) {
			on_save();
		}
	}
	return TRUE;
}

void SludgeApplication::setFilename(char* filename)
{
	int i, lastSlash, j;
	lastSlash = -1;
	for (i = 0; filename[i] != 0; i++) {
		currentFilename[i] = filename[i];
		if (filename[i] == '/')
			lastSlash = i;
	}
	currentFilename[i] = 0;

	for (j = lastSlash + 1; j <= i; j++) {
		currentShortname[j-lastSlash-1] = filename[j];
	}

	gtk_window_set_title (GTK_WINDOW(theWindow), currentShortname);
	fileChanged = FALSE;
}

void SludgeApplication::setFolderFromFilename(char* filename)
{
	int i, lastSlash, j;
	for (i = 0; filename[i] != 0; i++) {
		if (filename[i] == '/')
			lastSlash = i;
	}
	for (j = 0; j < lastSlash; j++) {
		currentFolder[j] = filename[j];
	}
	currentFolder[lastSlash] = 0;
}

void SludgeApplication::setFileChanged()
{
	if (!fileChanged) {
		if (currentShortname[0] == 0) {
			sprintf (currentShortname, "%s", getUntitledFilename());
		}
		gtk_window_set_title (GTK_WINDOW(theWindow), joinTwoStrings("*", currentShortname));
		fileChanged = TRUE;
	}
}

void SludgeApplication::open(char* filename)
{
	if (!loadFile (filename))
	{
		errorBox ("Error", "Loading file failed.");
	}
	else
	{
		setFilename(filename);
		setFolderFromFilename(filename);
		postOpen ();
	}
}

// Callbacks:

gboolean SludgeApplication::on_window1_delete_event()
{
	if (reallyClose()) {
		gtk_main_quit ();
		return FALSE;
	} else {
		return TRUE;
	}
}

void SludgeApplication::on_new()
{
	if (!reallyClose()) return;
	init();
	currentFilename[0] = 0;
	sprintf (currentShortname, "%s", getUntitledFilename());
	gtk_window_set_title (GTK_WINDOW(theWindow), getWindowTitle());
	fileChanged = FALSE;

	postNew ();
}

void SludgeApplication::on_open()
{
	if (!reallyClose()) return;
	GtkWidget *dialog;
	GtkFileFilter *filter;

	dialog = gtk_file_chooser_dialog_new ("Open File",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, getFilterName());
	gtk_file_filter_add_pattern(filter, getFilterPattern());
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (dialog), filter);

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		flipBackslashes(&filename);
		open(filename);
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}

void SludgeApplication::on_save()
{
	if (currentFilename[0] != 0) {
		if (!saveFile (currentFilename))
		{
			errorBox ("Error", "Saving file failed.");
		} else {
			gtk_window_set_title (GTK_WINDOW(theWindow), currentShortname);
			fileChanged = FALSE;
		}
	}
	else
	{
		saveToFile();
	}
}

void SludgeApplication::on_save_as()
{
	saveToFile();
}

void SludgeApplication::on_about()
{
	const gchar *authors[] = { "Tim Furnish", "Rikard Peterson", "Tobias Hansen <tobias.han@gmx.de>", NULL }; 

	gtk_show_about_dialog(  GTK_WINDOW(theWindow),
							"program-name", getWindowTitle(),
							"authors", authors,
							"copyright", "© 2000-2011 Hungry Software and contributors",
							"license", 
"This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.",
							"version", "2.1.0",
							"website", "http://opensludge.sourceforge.net",
							"website-label", "Website",
							NULL);
}
