/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeFloorMaker.cpp - Part of the SLUDGE Floor Maker (GTK+ version)
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

#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <errno.h>
#include <sys/types.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "floormaker.h"
#include "sprites.h"
#include "splitter.hpp"
#include "Common.h"

#include "SludgeFloorMaker.h"

SludgeFloorMaker::SludgeFloorMaker()
 : SludgeGLApplication(joinTwoStrings(DATADIR, "FloorMaker.glade"), "floorIcon", "floormaker")
{
	if (!initSuccess) return;
	init(TRUE);
}

SludgeFloorMaker::~SludgeFloorMaker()
{
	if (backdrop.total) {
		forgetSpriteBank(&backdrop);
		backdrop.total = 0;
		noFloor(&firstPoly);
	}
}

// Concrete methods for SludgeApplication:

gboolean SludgeFloorMaker::init(gboolean calledFromConstructor) 
{
	firstPoly = 0;
	noFloor(&firstPoly);
		
	backdrop.total=0;
	backdrop.type=2;
	backdrop.sprites=NULL;
	backdrop.myPalette.pal = NULL;
	backdrop.myPalette.r=NULL;
	backdrop.myPalette.g=NULL;
	backdrop.myPalette.b=NULL;

	if (!calledFromConstructor) {
		prepareOpenGL();
	}

	if (!reserveSpritePal(&backdrop.myPalette, 0)) {
			return TRUE;
	}

    return FALSE;
}

const char * SludgeFloorMaker::getWindowTitle()
{
	return "SLUDGE Floor Maker";
}

const char * SludgeFloorMaker::getFilterName()
{
	return "SLUDGE Floor Files (*.flo)";
}

const char * SludgeFloorMaker::getFilterPattern()
{
	return "*.[fF][lL][oO]";
}

const char * SludgeFloorMaker::getUntitledFilename()
{
	return "Untitled Floor.flo";
}

gboolean SludgeFloorMaker::saveFile(char *filename)
{
	return saveFloorToFile(filename, &firstPoly);
}

gboolean SludgeFloorMaker::loadFile(char *filename)
{
	return loadFloorFromFile(filename, &firstPoly);
}

void SludgeFloorMaker::postOpen() {}

void SludgeFloorMaker::postNew() {}

// Concrete methods for SludgeGLApplication:

void SludgeFloorMaker::button1Press(int local_pointx, int local_pointy)
{
    	gboolean keepOn = TRUE;
		int i, xx, yy;

		mouseLoc1x = local_pointx;
		mouseLoc1y = local_pointy;

		xx = (local_pointx+x)*zmul;
		yy = (local_pointy-y)*zmul;
		
		switch (mode) {
		case 0: // Define floor border
			snapToClosest(&xx, &yy, getFloor());
			while (keepOn) {
				keepOn = FALSE;
				i = addVertex(xx, yy, getFloor());
				switch (i) {
					case 1:
						setFileChanged();
						return;
					case 0:
						errorBox("Can't add vertex", "Out of memory.");
						return;
						
					case 3:
						errorBox("Can't add vertex", "That vertex is already used in this polygon, but isn't the start point.");
						return;
						
					case 2:
						if ( askAQuestion("Can't add vertex", "Can't add another vertex as the floor is already complete... do you want to start a NEW polygon at this point?") ) { 
							setFloor( addPoly(getFloor()) );
							keepOn = TRUE;
							setFileChanged();
						} else {
							return;
						}
						break;
				}
			}
			break;
		case 1: // Move vertices
		case 4: // Split lines
		case 5: // Split segments
			if (! snapToClosest(&xx, &yy, getFloor()))
				return;
			
			selx1 = xx;
			sely1 = yy;
			selection = 1;
			awaitButton1Release = TRUE;
				
			break;
		case 2: // Remove vertices
			if (! snapToClosest(&xx, &yy, getFloor()))
				return;
			struct polyList * firstPoly = getFloor();
			
			killVertex(xx, yy, &firstPoly);
			setFloor(firstPoly);
			setFileChanged();
			break;
		}
		lit = snapToClosest(&xx, &yy, getFloor());
		litX = xx; litY = yy;
}

void SludgeFloorMaker::button1Release(int local_pointx, int local_pointy)
{
		int xx, yy;

		awaitButton1Release = FALSE;

		selx2 = xx = (local_pointx+x)*zmul;
		sely2 = yy = (local_pointy-y)*zmul;

		lit = snapToClosest(&xx, &yy, getFloor());
		litX = xx; 
		litY = yy;
						
		if (lit && (xx != selx1 || yy != sely1)) {
			selx2 = xx;
			sely2 = yy;
		}
												
		selection = 0;

		switch (mode) {
			case 1: // Move vertices
				if (moveVertices(selx1, sely1, selx2, sely2, getFloor())) {
					setFileChanged();
				} else {
					errorBox("Can't move vertex", "Sorry - that vertex is already contained in one or more of the polygons you're changing.");
					return;
				}
			
				break;

			case 4: // Split lines
				splitLine(selx1, sely1, selx2, sely2, getFloor());
				setFileChanged();
				break;
			case 5: // Split segments
				struct polyList * firstPoly = getFloor();
				splitPoly(selx1, sely1, selx2, sely2, &firstPoly);
				setFloor(firstPoly);
				setFileChanged();
				break;
		}
}

void SludgeFloorMaker::button1Motion(int local_pointx, int local_pointy)
{
		int xx, yy;

		selx2 = xx = (local_pointx+x)*zmul;
		sely2 = yy = (local_pointy-y)*zmul;

		lit = snapToClosest(&xx, &yy, getFloor());
		litX = xx; 
		litY = yy;
						
		if (lit && (xx != selx1 || yy != sely1)) {
			selx2 = xx;
			sely2 = yy;
		}
}

void SludgeFloorMaker::drawingareaLeft()
{
	selection = 0;
}

void SludgeFloorMaker::prepareOpenGL()
{
	if (! backdrop.total)
		addSprite(0, &backdrop);
	z = 0.0;
	r = g = b = 1.0;
	zmul = (1.0+z/20);
	setCoords();	
}

void SludgeFloorMaker::drawRect()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
	
	glColor3f(1.0f, 1.00f, 1.00f);
	pasteSprite(&backdrop.sprites[0], &backdrop.myPalette, FALSE);

	glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 0.35f, 0.35f);
    glBegin(GL_LINES);
    {
        glVertex3f(  0.0,  0.0, 0.0);
        glVertex3f(  1000.0,  0.0, 0.0);
        glVertex3f(  0.0, 0.0, 0.0);
        glVertex3f(  0.0, 1000.0, 0.0);
    }
    glEnd();

	if (lit) {
		glColor3f(1.0f, 0.00f, 1.00f);
		glBegin(GL_QUADS);
		{
			glVertex3i(  litX-8,  litY-8, 0);
			glVertex3i(  litX+8,  litY-8, 0);
			glVertex3i(  litX+8,  litY+8, 0);
			glVertex3i(  litX-8,  litY+8, 0);
		}
		glEnd();
	}
	
	drawFloor(getFloor(), r, g, b);
	
	if (selection == 1) {
		glColor3f(1.0f, 0.00f, 1.00f);
		glBegin(GL_LINES);
		{
			glVertex3i(selx1, sely1, 0);
			glVertex3i(selx2, sely2, 0);
		}
		glEnd();
	}
}

struct polyList * SludgeFloorMaker::getFloor() {
	return firstPoly;
}

void SludgeFloorMaker::setFloor(struct polyList * floor)
{
	firstPoly = floor;
}

// Callbacks:

void SludgeFloorMaker::on_filechooserbutton_realize(GtkFileChooser *theChooser)
{	
	setFileChooserFilters(theChooser, TRUE, TRUE);

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder(theChooser, currentFolder);
	}
}

void SludgeFloorMaker::on_filechooserbutton_file_set(GtkFileChooser *theChooser)
{	
	char *filename;
	gboolean success = 0;

    filename = gtk_file_chooser_get_filename(theChooser);
	if (filename == NULL) return;

	flipBackslashes(&filename);

	if (strlen(filename) > 4) {
		char * extension = filename + strlen(filename) - 4;
		if        (!strcmp(extension, ".png") || !strcmp(extension, ".PNG")) {
			success = loadSpriteFromPNG(filename, &backdrop, 0);
		} else if (!strcmp(extension, ".tga") || !strcmp(extension, ".TGA")) {
			success = loadSpriteFromTGA(filename, &backdrop, 0);
		}
	} else {
		errorBox("Can't load image", "I don't recognise the file type. TGA and PNG are the supported file types.");
	}

	if (success) {
		setFolderFromFilename(filename);
		backdrop.sprites[0].height = -backdrop.sprites[0].height;
		activateZoomButtons(backdrop.sprites[0].width, -backdrop.sprites[0].height);
		showStatusbar(backdrop.sprites[0].width, -backdrop.sprites[0].height);
		on_zoom_fit_clicked();
		reshape();
		render_timer_event(theDrawingarea);
	}

	g_free(filename);
}

void SludgeFloorMaker::on_colorbutton_realize(GtkColorButton *theButton)
{	
	GdkColor theColor;
	theColor.red = theColor.blue = theColor.green = 65535;
	gtk_color_button_set_color(theButton, &theColor);
}

void SludgeFloorMaker::on_colorbutton_color_set(GtkColorButton *theButton)
{	
	GdkColor theColor;
	gtk_color_button_get_color(theButton, &theColor);
	r = theColor.red / 65535.;
	g = theColor.green / 65535.;
	b = theColor.blue / 65535.;
}

void SludgeFloorMaker::on_mode_changed(int buttonClicked)
{
	mode = buttonClicked;
}
