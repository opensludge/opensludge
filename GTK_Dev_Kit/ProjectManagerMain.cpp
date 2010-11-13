/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ProjectManagerMain.cpp - Part of the SLUDGE Project Manager (GTK+ version)
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

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib-object.h>
#include <glib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <getopt.h>

#include "ProjectManagerMain.h"
#include "SludgeProjectManager.h"
#include "Common.h"

SludgeProjectManager *projectManager;

#ifdef __cplusplus
extern "C" {
#endif

// SludgeApplication callbacks:

G_MODULE_EXPORT gboolean on_window1_delete_event(GtkWidget *theWidget, GdkEvent *theEvent, gpointer theUser_data)
{
	return projectManager->on_window1_delete_event();
}

G_MODULE_EXPORT void on_new(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_new();
}

G_MODULE_EXPORT void on_open(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_open();
}

G_MODULE_EXPORT void on_save(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_save();
}

G_MODULE_EXPORT void on_save_as(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_save_as();
}

G_MODULE_EXPORT void
on_about(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_about();
}

// SludgeProjectManager callbacks:

G_MODULE_EXPORT void on_files_treeview_realize(GtkTreeView *theTreeView, gpointer theUser_data)
{
	projectManager->on_treeview_realize(theTreeView, FILE_TREEVIEW);
}

G_MODULE_EXPORT void on_resources_treeview_realize(GtkTreeView *theTreeView, gpointer theUser_data)
{
	projectManager->on_treeview_realize(theTreeView, RESOURCE_TREEVIEW);
}

G_MODULE_EXPORT void on_errors_treeview_realize(GtkTreeView *theTreeView, gpointer theUser_data)
{
	projectManager->on_treeview_realize(theTreeView, ERROR_TREEVIEW);
}

G_MODULE_EXPORT void
on_files_tree_selection_changed_cb(GtkTreeSelection *theSelection, gpointer theUser_data)
{
	projectManager->on_files_tree_selection_changed(theSelection);
}

G_MODULE_EXPORT void
on_files_treeview_row_activated(GtkTreeView *theTreeView, GtkTreePath *thePath, GtkTreeViewColumn *theColumn, gpointer theUser_data)
{
	projectManager->on_treeview_row_activated(theTreeView, thePath, theColumn, FILE_TREEVIEW);
}

G_MODULE_EXPORT void
on_resources_treeview_row_activated(GtkTreeView *theTreeView, GtkTreePath *thePath, GtkTreeViewColumn *theColumn, gpointer theUser_data)
{
	projectManager->on_treeview_row_activated(theTreeView, thePath, theColumn, RESOURCE_TREEVIEW);
}

G_MODULE_EXPORT void
on_errors_treeview_row_activated(GtkTreeView *theTreeView, GtkTreePath *thePath, GtkTreeViewColumn *theColumn, gpointer theUser_data)
{
	projectManager->on_treeview_row_activated(theTreeView, thePath, theColumn, ERROR_TREEVIEW);
}

G_MODULE_EXPORT void on_add_file_clicked(GtkButton *theButton, gpointer theUser_data)
{
	projectManager->on_add_file_clicked();
}

G_MODULE_EXPORT void on_remove_file_clicked(GtkButton *theButton, gpointer theUser_data)
{
	projectManager->on_remove_file_clicked();
}

G_MODULE_EXPORT void on_compile(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_compile();
}

G_MODULE_EXPORT void on_comp_okbutton_clicked(GtkButton *theButton, gpointer theUser_data)
{
	projectManager->on_comp_okbutton_clicked(theButton);
}

G_MODULE_EXPORT void on_comp_gamebutton_clicked(GtkButton *theButton, gpointer theUser_data)
{
	projectManager->on_comp_gamebutton_clicked();
}

G_MODULE_EXPORT void on_project_settings(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_project_settings();
}

G_MODULE_EXPORT void on_preferences(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_preferences();
}

G_MODULE_EXPORT void on_floormaker_activate(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_program_activate(FLOORMAKER);
}

G_MODULE_EXPORT void on_spritebankeditor_activate(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_program_activate(SPRITEBANKEDITOR);
}

G_MODULE_EXPORT void on_zbuffermaker_activate(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_program_activate(ZBUFFERMAKER);
}

G_MODULE_EXPORT void on_translationeditor_activate(GtkMenuItem *theItem, gpointer theUser_data)
{
	projectManager->on_program_activate(TRANSLATIONEDITOR);
}

G_MODULE_EXPORT void *compile_hook(gpointer nothing)
{
	projectManager->compile();
}

G_MODULE_EXPORT gboolean update_compile_window_hook(gpointer nothing)
{
	projectManager->update_compile_window();
	return FALSE;
}

void receiveCompilerInfo(compilerInfo *info)
{
	g_async_queue_push(projectManager->compilerInfoQueue, info);
	g_idle_add(update_compile_window_hook, NULL);
}

G_MODULE_EXPORT gboolean on_project_settings_dialog_delete_event(GtkWidget *theWidget, GdkEvent  *theEvent, gpointer theUser_data)
{
	return gtk_widget_hide_on_delete(theWidget);
}

G_MODULE_EXPORT gboolean on_preferences_dialog_delete_event(GtkWidget *theWidget, GdkEvent  *theEvent, gpointer theUser_data)
{
	return gtk_widget_hide_on_delete(theWidget);
}

G_MODULE_EXPORT gboolean on_compiler_dialog_delete_event(GtkWidget *theWidget, GdkEvent  *theEvent, gpointer theUser_data)
{
	return gtk_widget_hide_on_delete(theWidget);
}
#ifdef __cplusplus
}
#endif

void printCmdlineUsage() {
	fprintf(stdout, "SLUDGE Project Manager, usage: sludge-projectmanager <project file>\n");
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
		char c = getopt_long(argc, argv, "h", long_options, &option_index);
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


/*
 * The main function.
 */
int
main(int argc, char *argv[])
{
	char *openThisFile = argv[argc - 1];

	if (! parseCmdlineParameters(argc, argv) ) {
		printCmdlineUsage();
		return 0;
	}

	if (argc < 2) {
		openThisFile = NULL;
	} else if (! fileExists(openThisFile) ) {
		errorBox("Project file not found!", joinTwoStrings("File not found:\n", openThisFile));
		printCmdlineUsage();
		return -1;
	}

	// On Windows, change to the program directory to
	// make sure the necessary resource files are found:
	winChangeToProgramDir(argv[0]);

	if (!g_thread_supported ()){ g_thread_init (NULL); }
	gdk_threads_init ();
	gdk_threads_enter ();

	/*
	 * Init GTK+.
	 */
	gtk_init(&argc, &argv);

	projectManager = new SludgeProjectManager();

	/*
	 * Get the window manager to connect any assigned signals in the loaded Glade file to our coded functions.
	 */
	gtk_builder_connect_signals(projectManager->theXml, NULL);

	/*
	 * Show the window.
	 */
	gtk_widget_show(projectManager->theWindow);

	if (openThisFile != NULL) {
		projectManager->open(openThisFile);
	}

	// Run the window manager loop.
	if (projectManager->initSuccess)
		gtk_main ();

	delete projectManager;

	gdk_threads_leave ();

	return 0;
}
