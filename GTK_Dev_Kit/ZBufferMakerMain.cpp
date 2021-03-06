/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ZBufferMakerMain.cpp - Part of the SLUDGE Z-Buffer Maker (GTK+ version)
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
#include <gtk/gtkgl.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib-object.h>
#include <glib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>

#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <getopt.h>

#include "SludgeZBufferMaker.h"
#include "Common.h"

SludgeZBufferMaker *zBufferMaker;

#ifdef __cplusplus
extern "C" {
#endif

// SludgeApplication callbacks:

G_MODULE_EXPORT gboolean on_window1_delete_event(GtkWidget *theWidget, GdkEvent *theEvent, gpointer theUser_data)
{
	return zBufferMaker->on_window1_delete_event();
}

G_MODULE_EXPORT void on_new(GtkMenuItem *theItem, gpointer theUser_data)
{
	zBufferMaker->on_new();
}

G_MODULE_EXPORT void on_open(GtkMenuItem *theItem, gpointer theUser_data)
{
	zBufferMaker->on_open();
}

G_MODULE_EXPORT void on_save(GtkMenuItem *theItem, gpointer theUser_data)
{
	zBufferMaker->on_save();
}

G_MODULE_EXPORT void on_save_as(GtkMenuItem *theItem, gpointer theUser_data)
{
	zBufferMaker->on_save_as();
}

G_MODULE_EXPORT void
on_about(GtkMenuItem *theItem, gpointer theUser_data)
{
	zBufferMaker->on_about();
}

// SludgeGLApplication callbacks:

G_MODULE_EXPORT void on_drawingarea1_realize(GtkWidget *theWidget, gpointer theUser_data)
{
	zBufferMaker->on_drawingarea1_realize(theWidget);
}

G_MODULE_EXPORT gboolean on_drawingarea1_configure_event(GtkWidget *theWidget, GdkEventConfigure *theEvent, gpointer theUser_data)
{
	return zBufferMaker->on_drawingarea1_configure_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean on_drawingarea1_expose_event(GtkWidget *theWidget, GdkEventExpose *theEvent, gpointer theUser_data)
{
	return zBufferMaker->on_drawingarea1_expose_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean render_timer_event(gpointer theUser_data)
{
	return zBufferMaker->render_timer_event(theUser_data);
}

G_MODULE_EXPORT gboolean on_drawingarea1_scroll_event(GtkWidget *theWidget, GdkEventScroll *theEvent, gpointer theUser_data)
{
	return zBufferMaker->on_drawingarea1_scroll_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean on_drawingarea1_button_press_event(GtkWidget *theWidget, GdkEventButton *theEvent, gpointer theUser_data)
{
    return zBufferMaker->on_drawingarea1_button_press_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean on_drawingarea1_button_release_event(GtkWidget *theWidget, GdkEventButton *theEvent, gpointer theUser_data)
{
    return zBufferMaker->on_drawingarea1_button_release_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean on_drawingarea1_motion_notify_event(GtkWidget *theWidget, GdkEventMotion *theEvent, gpointer theUser_data)
{
    return zBufferMaker->on_drawingarea1_motion_notify_event(theWidget, theEvent);
}

G_MODULE_EXPORT gboolean on_drawingarea1_leave_notify_event(GtkWidget *theWidget, GdkEventAny *theEvent, gpointer theUser_data)
{
    return zBufferMaker->on_drawingarea1_leave_notify_event(theWidget, theEvent);
}

G_MODULE_EXPORT void on_zoom_100_realize(GtkToolButton *theButton, gpointer theUser_data)
{
	zBufferMaker->on_zoom_100_realize(theButton);
}

G_MODULE_EXPORT void on_zoom_fit_realize(GtkToolButton *theButton, gpointer theUser_data)
{
	zBufferMaker->on_zoom_fit_realize(theButton);
}

G_MODULE_EXPORT void on_zoom_100_clicked(GtkToolButton *theButton, gpointer theUser_data)
{
	zBufferMaker->on_zoom_100_clicked();
}

G_MODULE_EXPORT void on_zoom_fit_clicked(GtkToolButton *theButton, gpointer theUser_data)
{
	zBufferMaker->on_zoom_fit_clicked();
}

// SludgeZBufferMaker callbacks:

G_MODULE_EXPORT void on_hscale_value_changed(GtkHScale *theScale, gpointer theUser_data)
{
	zBufferMaker->on_hscale_value_changed(theScale);
}

G_MODULE_EXPORT void on_spinbutton_value_changed(GtkSpinButton *theSpinButton, gpointer theUser_data)
{
	zBufferMaker->on_spinbutton_value_changed(theSpinButton);
}

#ifdef __cplusplus
}
#endif

void printCmdlineUsage() {
	fprintf(stdout, "SLUDGE Z-Buffer Maker, usage: sludge-zbuffermaker <z-buffer file>\n");
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
		int c = getopt_long(argc, argv, "h", long_options, &option_index);
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
		fprintf(stderr, "Z-Buffer file not found.\n");
		printCmdlineUsage();
		return -1;
	}

	// On Windows, change to the program directory to
	// make sure the necessary resource files are found:
	winChangeToProgramDir(argv[0]);

	if (!g_thread_supported()){ g_thread_init(NULL); }
	gdk_threads_init();
	gdk_threads_enter();

	/*
	 * Init GTK+ and GtkGLExt.
	 */
	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	zBufferMaker = new SludgeZBufferMaker();

	/*
	 * Get the window manager to connect any assigned signals in the loaded Glade file to our coded functions.
	 */
	gtk_builder_connect_signals(zBufferMaker->theXml, NULL);

	/*
	 * Show the window.
	 */
	gtk_widget_show(zBufferMaker->theWindow);

	// Start the render timer.
	g_timeout_add(10000 / 60, render_timer_event, zBufferMaker->theDrawingarea);

	if (openThisFile != NULL) {
		zBufferMaker->open(openThisFile);
	}

	// Run the window manager loop.
	if (zBufferMaker->initSuccess)
		gtk_main();

	delete zBufferMaker;

	gdk_threads_leave();

	return 0;
}
