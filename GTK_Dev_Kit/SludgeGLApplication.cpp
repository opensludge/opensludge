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

#include "SludgeGLApplication.h"


SludgeGLApplication::SludgeGLApplication(const char * gladeFileName, const char * iconName, const char * configFile)
 : SludgeApplication(gladeFileName, iconName, configFile)
{
	if (!initSuccess) return;

	glInitialized = FALSE;

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

	x = y = w = h = 0;

	theDrawingarea = GTK_WIDGET (gtk_builder_get_object (theXml, "drawingarea1"));

	if (theDrawingarea == NULL)
	{
		g_critical ("Failed to get the drawing area from the builder.\n");
		initSuccess = FALSE;
		return;
	}

	// Configure the OpenGL drawing area. GtkGLArea creates and manages the
	// context, and swaps the buffers after every render.
	gtk_gl_area_set_has_depth_buffer (GTK_GL_AREA (theDrawingarea), TRUE);
	gtk_gl_area_set_has_alpha (GTK_GL_AREA (theDrawingarea), TRUE);
	gtk_gl_area_set_auto_render (GTK_GL_AREA (theDrawingarea), TRUE);

	// Initialise the render mutex.
	g_mutex_init (&theRender_mutex);

	haveStatusbar = FALSE;
	zmul = 1.;
}

SludgeGLApplication::~SludgeGLApplication()
{
	g_mutex_clear (&theRender_mutex);
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
	gtk_widget_hide(statusbarWidget);
}

void SludgeGLApplication::reshape()
{
	int newW = gtk_widget_get_allocated_width (theDrawingarea);
	int newH = gtk_widget_get_allocated_height (theDrawingarea);
	if (! w) x = 0;
	else x -= (newW-w)/2;
	if (! h) y = 0;
	else y += (newH-h)/2;
	h = newH;
	w = newW;

	if (!glInitialized) return;

	glViewport (0, 0, w, h);	
	
	setCoords();
}

/*
 * Make sure the OpenGL context exists and is current, initialise GLEW and let
 * the application set up its OpenGL state. Nothing happens until the widget is
 * realized and its context is available; instead the work is deferred to the
 * first "render" signal.
 */
void SludgeGLApplication::initGL()
{
	if (glInitialized) return;
	if (theDrawingarea == NULL) return;
	if (gtk_gl_area_get_context (GTK_GL_AREA (theDrawingarea)) == NULL) return;

	gtk_gl_area_make_current (GTK_GL_AREA (theDrawingarea));
	if (gtk_gl_area_get_error (GTK_GL_AREA (theDrawingarea)) != NULL) return;

	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
	{
		g_critical ("Couldn't initialize GLEW.\n");
		return;
	}

	prepareOpenGL();
	glInitialized = TRUE;
}

/*
 * Ask for the OpenGL state to be set up again, for instance after a new image
 * was loaded. If the context is ready this happens immediately, otherwise it is
 * deferred to the next render.
 */
void SludgeGLApplication::refreshGL()
{
	glInitialized = FALSE;
	initGL();
	if (theDrawingarea != NULL) gtk_widget_queue_draw (theDrawingarea);
}

void SludgeGLApplication::setCoords()
{
	if (!glInitialized) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (-y+h)*zmul, -y*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}

// Callbacks:

void SludgeGLApplication::on_drawingarea1_realize(GtkWidget *theWidget)
{
	if (gtk_widget_get_window (theWidget) == NULL)
	{
		return;
	}

	gtk_widget_add_events(theWidget, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
					 | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_SCROLL_MASK);  

	// Set up OpenGL now if the context is already available.
	initGL();
}

/*
 * The "render" signal handler for drawingarea1. All OpenGL re-drawing is done here.
 * GtkGLArea makes the OpenGL context current and swaps the buffers for us.
 */
gboolean SludgeGLApplication::on_drawingarea1_render(GtkWidget *theWidget, GdkGLContext *context)
{
	// First frame: initialise GLEW and the application's OpenGL state.
	initGL();

	/*
	 * Lock rendering mutex if it's not busy - otherwise exit.
	 * This prevents the renderer being spammed by drawing requests if the rendering takes longer than the feed/timer process.
	 */
	if (!g_mutex_trylock(&theRender_mutex))
		return TRUE;

	if (glInitialized)
	{
		reshape();
		drawRect();
	}

	// Release the rendering mutex.
	g_mutex_unlock(&theRender_mutex);

	return TRUE;
}

/*
 * The timer-based process tells GtkGLArea to redraw, which results in another "render" signal.
 */
gboolean SludgeGLApplication::render_timer_event(gpointer theUser_data)
{
	GtkWidget *theWidget = GTK_WIDGET (theUser_data);

	/*
	 * Only send a redraw request if the render process is not busy.  This prevents long rendering processes
	 * from piling up.
	 */
	if (g_mutex_trylock(&theRender_mutex))
	{
		// Unlock the mutex before we issue the redraw.
		g_mutex_unlock(&theRender_mutex);
		gtk_widget_queue_draw (theWidget);
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

	if ( (awaitButton1Release && theEvent->state & GDK_BUTTON1_MASK) || awaitButton2Release )
	{
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
	}

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
