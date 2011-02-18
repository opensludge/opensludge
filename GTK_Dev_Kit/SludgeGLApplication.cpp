/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeGLApplication.cpp - Part of the SLUDGE Dev Kit (GTK+ version)
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

#include <GLee.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "SludgeGLApplication.h"


SludgeGLApplication::SludgeGLApplication(const char * gladeFileName, const char * iconName, const char * configFile)
 : SludgeApplication(gladeFileName, iconName, configFile)
{
	if (!initSuccess) return;

	pixmap = NULL;
	glpixmap = NULL;
	context = NULL;
	glDrawable = NULL;
	glContext = NULL;

	statusbarWidget = GTK_WIDGET (gtk_builder_get_object (theXml, "statusbar"));
	cursorxLabel = GTK_LABEL (gtk_builder_get_object (theXml, "cursorx_label"));
	cursoryLabel = GTK_LABEL (gtk_builder_get_object (theXml, "cursory_label"));
	picwLabel = GTK_LABEL (gtk_builder_get_object (theXml, "picw_label"));
	pichLabel = GTK_LABEL (gtk_builder_get_object (theXml, "pich_label"));
	zmulLabel = GTK_LABEL (gtk_builder_get_object (theXml, "zmul_label"));
	zoom100Button = NULL;
	zoomFitButton = NULL;

	awaitButton1Release = FALSE;
	awaitButton2Release = FALSE;

	GdkGLConfig *glConfig;

	/*
	 * Configure a OpenGL-capable context.
	 */
	// Try to make it double-buffered.
	glConfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
		(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA | GDK_GL_MODE_DOUBLE));
	if (glConfig == NULL)
	{
		g_print ("Cannot configure a double-buffered context.\n");
		g_print ("Will try a single-buffered context.\n");

		// If we can't configure a double-buffered context, try for single-buffered.
		glConfig = gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode>
		(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA));
		if (glConfig == NULL)
		{
			g_critical ("Aargh!  Cannot configure any type of OpenGL-capable context. Exiting.\n");
			initSuccess = FALSE;
			return;
		}
	}

	theDrawingarea = GTK_WIDGET (gtk_builder_get_object (theXml, "drawingarea1"));

	// Add OpenGL-capability to the drawing area.
	if (!gtk_widget_set_gl_capability (theDrawingarea, glConfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
	{
		g_critical ("Cannot set OpenGL-capability to widget. Exiting.\n");
		initSuccess = FALSE;
		return;
	}

	// Initialise the render mutex.
	theRender_mutex = g_mutex_new();

	haveStatusbar = FALSE;
	zmul = 1.;
}

SludgeGLApplication::~SludgeGLApplication()
{
	g_mutex_free(theRender_mutex);
}

void SludgeGLApplication::setZ(float newZ)
{
	z = newZ;
	if (z > 200.0) z = 200.0;
	if (z < -19.0) z = -19.0;
	zmul = (1.0+z/20);
	refreshStatusbarZmul();
}

void
SludgeGLApplication::activateZoomButtons(int picwidth, int picheight)
{
	picWidth = picwidth;
	picHeight = picheight;
	gtk_widget_set_sensitive (GTK_WIDGET(zoom100Button), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET(zoomFitButton), TRUE);
}

void
SludgeGLApplication::deactivateZoomButtons()
{
	if (zoom100Button)
		gtk_widget_set_sensitive (GTK_WIDGET(zoom100Button), FALSE);
	if (zoomFitButton)
		gtk_widget_set_sensitive (GTK_WIDGET(zoomFitButton), FALSE);
}

void SludgeGLApplication::showStatusbar(int picwidth, int picheight)
{
	picWidth = picwidth;
	picHeight = picheight;
	haveStatusbar = TRUE;
	char buf[100];
	sprintf(buf, "%i", picWidth);
	gtk_label_set_text (picwLabel, buf);
	sprintf(buf, "%i", picHeight);
	gtk_label_set_text (pichLabel, buf);
	refreshStatusbarCursor(0, 0);
	refreshStatusbarZmul();
	gtk_widget_show_all(statusbarWidget);
}

void SludgeGLApplication::refreshStatusbarCursor(int local_pointx, int local_pointy)
{
	if (!haveStatusbar) return;

	char buf[100];
	sprintf(buf, "%i", (int)((local_pointx+x)*zmul));
	gtk_label_set_text (cursorxLabel, buf);
	sprintf(buf, "%i", (int)((local_pointy-y)*zmul));
	gtk_label_set_text (cursoryLabel, buf);
}

void SludgeGLApplication::refreshStatusbarZmul()
{
	if (!haveStatusbar) return;

	char buf[100];
	sprintf(buf, "%i", (int)(100/zmul));
	gtk_label_set_text (zmulLabel, buf);
}

void SludgeGLApplication::hideStatusbar()
{
	gtk_widget_hide_all(statusbarWidget);
	gtk_widget_show(statusbarWidget);
}

void SludgeGLApplication::reshape()
{
	if (! w) x = 0;
	else x -= (theDrawingarea->allocation.width-w)/2;
	if (! h) y = 0;
	else y += (theDrawingarea->allocation.height-h)/2;
	h = theDrawingarea->allocation.height;
	w = theDrawingarea->allocation.width;
	glViewport (0, 0, w, h);	
	
	setCoords();
}

void SludgeGLApplication::setCoords()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (-y+h)*zmul, -y*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}

// Callbacks:

void SludgeGLApplication::on_drawingarea1_realize(GtkWidget *theWidget)
{
	GdkGLContext *glContext;
	GdkGLDrawable *glDrawable;

	if (theWidget->window == NULL)
	{
		return;
	}

	gtk_widget_add_events(theWidget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
					 | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_SCROLL_MASK);  

	glContext = gtk_widget_get_gl_context (theWidget);
	glDrawable = gtk_widget_get_gl_drawable (theWidget);

	// Signal to gdk the start OpenGL operations.
	if (!gdk_gl_drawable_gl_begin (glDrawable, glContext))
	{
		return;
	}

	glViewport (0, 0, theWidget->allocation.width, theWidget->allocation.height);

	/* Your one-time OpenGL initialization code goes here */
    x = y = 0;
    w = theWidget->allocation.width;
    h = theWidget->allocation.height;
	prepareOpenGL();

	// Signal to gdk we're done with OpenGL operations.
	gdk_gl_drawable_gl_end (glDrawable);

	gdk_window_invalidate_rect (gtk_widget_get_parent_window (theWidget), &theWidget->allocation, TRUE);
}

/*
 * The "configure_event" signal handler for drawingarea1. When the area is reconfigured (such as a resize)
 * this gets called - so we change the OpenGL viewport accordingly,
 */
gboolean SludgeGLApplication::on_drawingarea1_configure_event (GtkWidget *theWidget, GdkEventConfigure *theEvent)
{
GdkGLContext *glContext;
GdkGLDrawable *glDrawable;


	// Don't continue if we get fed a dud window type.
	if (theWidget->window == NULL)
	{
		return FALSE;
	}

	/*
	 * Wait until the rendering process is complete before issuing a context change,
	 * then lock it - to prevent possible driver issues on long rendering processes.
	 * You can add a timeout here or g_mutex_trylock according to taste.
	 */
	g_mutex_lock(theRender_mutex);

	glContext = gtk_widget_get_gl_context (theWidget);
	glDrawable = gtk_widget_get_gl_drawable (theWidget);

	// Signal to gdk the start OpenGL operations.
	if (!gdk_gl_drawable_gl_begin (glDrawable, glContext))
	{
		g_mutex_unlock(theRender_mutex);
		return FALSE;
	}

/*	glViewport (0, 0, theWidget->allocation.width, theWidget->allocation.height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 20.0);
	glMatrixMode (GL_MODELVIEW);
*/
    reshape();

	glFlush ();

	// Signal to gdk we're done with OpenGL operations.
	gdk_gl_drawable_gl_end (glDrawable);

	// Unlock the rendering mutex before sending an immediate redraw request.
	g_mutex_unlock(theRender_mutex);
	gdk_window_invalidate_rect (gtk_widget_get_parent_window (theWidget), &theWidget->allocation, TRUE);
	gdk_window_process_updates (gtk_widget_get_parent_window (theWidget), TRUE);
//	gdk_window_invalidate_rect (theWidget->window, &theWidget->allocation, FALSE);
//	gdk_window_process_updates (theWidget->window, FALSE);

	return FALSE;
}

/*
 * The "expose_event" signal handler for drawingarea1. All OpenGL re-drawing should be done here.
 * This is called every time the expose/draw event is signalled.
 *
 */
gboolean SludgeGLApplication::on_drawingarea1_expose_event(GtkWidget *theWidget, GdkEventExpose *theEvent)
{
	GdkGLContext *glContext;
	GdkGLDrawable *glDrawable;

	// Don't continue if we get fed a dud window type.
	if (theWidget->window == NULL)
	{
		return FALSE;
	}

	/*
	 * Lock rendering mutex if it's not busy - otherwise exit.
	 * This prevents the renderer being spammed by drawing requests if the rendering takes longer than the feed/timer process.
	 */
	if (!g_mutex_trylock(theRender_mutex))
		return FALSE;

	glContext = gtk_widget_get_gl_context (theWidget);
	glDrawable = gtk_widget_get_gl_drawable (theWidget);

	// Signal to gdk the start OpenGL operations.
	if (!gdk_gl_drawable_gl_begin (glDrawable, glContext))
	{
		g_mutex_unlock(theRender_mutex);
		return FALSE;
	}

	drawRect();

	/*
	 * Swap rendering buffers out (an auto glFlush is issued after) if we previously managed to initialise
	 * it as a double-buffered context, otherwise issue a standard glFlush call to signal we've finished
	 * OpenGL rendering.
	 */
	if (gdk_gl_drawable_is_double_buffered (glDrawable))
		gdk_gl_drawable_swap_buffers (glDrawable);
	else
		glFlush ();

	// Signal to gdk we're done with OpenGL operations.
	gdk_gl_drawable_gl_end (glDrawable);

	// Release the rendering mutex.
	g_mutex_unlock(theRender_mutex);

	return FALSE;
}

/*
 * The timer-based process that operates on some variables then invalidates drawingarea1's region to trigger
 * an expose_event, thus invoking function "on_drawingarea1_expose_event" through the window manager.
 *
 */
gboolean SludgeGLApplication::render_timer_event(gpointer theUser_data)
{
	GtkWidget *theWidget = GTK_WIDGET (theUser_data);

	/*
	 * Only send a redraw request if the render process is not busy.  This prevents long rendering processes
	 * from piling up.
	 */
	if (g_mutex_trylock(theRender_mutex))
	{
		// Unlock the mutex before we issue the redraw.
		g_mutex_unlock(theRender_mutex);
		// Invalidate drawingarea1's region to signal the window handler there needs to be an update in that area,
		gdk_window_invalidate_rect (gtk_widget_get_parent_window (theWidget), &theWidget->allocation, TRUE);
//		gdk_window_invalidate_rect (theWidget->window, &theWidget->allocation, FALSE);
		/*
		 * Force an immediate update - we can omit this and leave the window manager to call the redraw when the
		 * main loop is idle.  However, we get smoother, more consistent updates this way.
		 */
		gdk_window_process_updates (gtk_widget_get_parent_window (theWidget), TRUE);
//		gdk_window_process_updates (theWidget->window, FALSE);
	}

	return TRUE;
}

gboolean SludgeGLApplication::on_drawingarea1_scroll_event(GtkWidget *theWidget, GdkEventScroll *theEvent)
{
	float deltaY;
	if (theEvent->direction == GDK_SCROLL_UP) {
		deltaY = -1.;
	}
	if (theEvent->direction == GDK_SCROLL_DOWN) {
		deltaY = 1.;
	}
	double x1, y1;
	int local_pointx, local_pointy;
	
	local_pointx = theEvent->x;
	local_pointy = theEvent->y;
	x1 = zmul*(local_pointx+x);
	y1 = -zmul*(local_pointy-y);

	setZ(z+deltaY);

	x = -(local_pointx)+x1/zmul;
	y = (local_pointy)+y1/zmul;

	setCoords();
	render_timer_event(theDrawingarea);

	return FALSE;
}

gboolean SludgeGLApplication::on_drawingarea1_button_press_event
(GtkWidget *theWidget, GdkEventButton *theEvent)
{
	if (theEvent->button == 1) {
		button1Press (theEvent->x, theEvent->y);
	}
	else
	{
		mouseLoc2x = theEvent->x;
		mouseLoc2y = theEvent->y;
		awaitButton2Release = TRUE;	
	}

    return FALSE;
}

gboolean SludgeGLApplication::on_drawingarea1_button_release_event
(GtkWidget *theWidget, GdkEventButton *theEvent)
{
	if ( !((awaitButton1Release && theEvent->button == 1) || awaitButton2Release) ) return FALSE;

	if (theEvent->button == 1) 
	{
		button1Release (theEvent->x, theEvent->y);
	}
	else
	{
		awaitButton2Release = FALSE;
	}

	render_timer_event(theDrawingarea);

    return FALSE;
}

gboolean SludgeGLApplication::on_drawingarea1_motion_notify_event
(GtkWidget *theWidget, GdkEventMotion *theEvent)
{
	refreshStatusbarCursor(theEvent->x, theEvent->y);

	if ( !((awaitButton1Release && theEvent->state & GDK_BUTTON1_MASK) || awaitButton2Release) ) return FALSE;

	int local_pointx, local_pointy;
	local_pointx = theEvent->x;
	local_pointy = theEvent->y;

	if (theEvent->state & GDK_BUTTON1_MASK)
	{
		button1Motion (local_pointx, local_pointy);
	}
	else if (awaitButton2Release)
	{
		int x1 = x;
		int y1 = y;
		x = x1 + (mouseLoc2x - local_pointx);
		y = y1 + (local_pointy - mouseLoc2y);

		setCoords();
		mouseLoc2x = theEvent->x;
		mouseLoc2y = theEvent->y;
	}
	render_timer_event(theDrawingarea);
	gdk_event_request_motions (theEvent);

    return FALSE;
}

gboolean SludgeGLApplication::on_drawingarea1_leave_notify_event
(GtkWidget *theWidget, GdkEventAny *theEvent)
{
	awaitButton1Release = FALSE;
	awaitButton2Release = FALSE;

	drawingareaLeft ();

    return FALSE;
}

void SludgeGLApplication::on_zoom_100_realize (GtkToolButton *theButton)
{
	gtk_widget_set_sensitive (GTK_WIDGET(theButton), FALSE);
	zoom100Button = theButton;
}

void SludgeGLApplication::on_zoom_fit_realize (GtkToolButton *theButton)
{
	gtk_widget_set_sensitive (GTK_WIDGET(theButton), FALSE);
	zoomFitButton = theButton;
}

void SludgeGLApplication::on_zoom_100_clicked ()
{
	setZ(0.);
	x = -(w - picWidth / zmul) / 2.;
	y = (h - picHeight / zmul) / 2.;
	reshape();
}

void SludgeGLApplication::on_zoom_fit_clicked ()
{
	float zmulx, zmuly;
	zmulx = (float) picWidth / w;
	zmuly = (float) picHeight / h;

	if (zmulx > zmuly) {
		zmul = zmulx;
		x = 0;
		y = (h - picHeight / zmul) / 2.;
	} else {
		zmul = zmuly;
		x = -(w - picWidth / zmul) / 2.;
		y = 0;
	}
	z =	20. * (zmul - 1.);

	refreshStatusbarZmul();
	reshape();
}
