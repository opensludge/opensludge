/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeGLApplication.h - Part of the SLUDGE Dev Kit (GTK+ version)
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

#include "SludgeApplication.h"

class SludgeGLApplication : public SludgeApplication {

private:
	int mouseLoc2x, mouseLoc2y;
	gboolean haveStatusbar;
	GdkPixmap *pixmap;
	GdkGLPixmap *glpixmap;
	GdkGLContext *context;
	GdkGLDrawable *glDrawable;
	GdkGLContext *glContext;
	GtkToolButton *zoom100Button, *zoomFitButton;
	GtkWidget *statusbarWidget;
	GtkLabel *cursorxLabel, *cursoryLabel, *picwLabel, *pichLabel, *zmulLabel;
	GMutex * theRender_mutex;

protected:
	int x, y, w, h;
	float z, zmul;
	int picWidth, picHeight;
	gboolean awaitButton1Release, awaitButton2Release;

public:
	GtkWidget *theDrawingarea;


private:
	virtual void button1Press(int local_pointx, int local_pointy) = 0;
	virtual void button1Release(int local_pointx, int local_pointy) = 0;
	virtual void button1Motion(int local_pointx, int local_pointy) = 0;
	virtual void drawingareaLeft() = 0;
	virtual void prepareOpenGL() = 0;
	virtual void drawRect() = 0;

protected:
	void setZ(float newZ);
	void activateZoomButtons(int picwidth, int picheight);
	void deactivateZoomButtons();
	void showStatusbar(int picwidth, int picheight);
	void refreshStatusbarCursor(int local_pointx, int local_pointy);
	void refreshStatusbarZmul();
	void hideStatusbar();
	void reshape();
	void setCoords();

public:
	SludgeGLApplication(const char * gladeFileName, const char * iconName, const char * configFile);
	~SludgeGLApplication();
	// Callbacks:
	void on_drawingarea1_realize(GtkWidget *theWidget);
	gboolean on_drawingarea1_configure_event(GtkWidget *theWidget, GdkEventConfigure *theEvent);
	gboolean on_drawingarea1_expose_event(GtkWidget *theWidget, GdkEventExpose *theEvent);
	gboolean render_timer_event(gpointer theUser_data);
	gboolean on_drawingarea1_scroll_event(GtkWidget *theWidget, GdkEventScroll *theEvent);
	gboolean on_drawingarea1_button_press_event(GtkWidget *theWidget, GdkEventButton *theEvent);
	gboolean on_drawingarea1_button_release_event(GtkWidget *theWidget, GdkEventButton *theEvent);
	gboolean on_drawingarea1_motion_notify_event(GtkWidget *theWidget, GdkEventMotion *theEvent);
	gboolean on_drawingarea1_leave_notify_event(GtkWidget *theWidget, GdkEventAny *theEvent);
	void on_zoom_100_realize(GtkToolButton *theButton);
	void on_zoom_fit_realize(GtkToolButton *theButton);
	void on_zoom_100_clicked();
	void on_zoom_fit_clicked();
};
