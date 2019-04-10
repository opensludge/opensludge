/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeZBufferMaker.cpp - Part of the SLUDGE Z-Buffer Maker (GTK+ version)
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

#include "zbuffer.h"
#include "Common.h"

#include "SludgeZBufferMaker.h"

SludgeZBufferMaker::SludgeZBufferMaker()
 : SludgeGLApplication(joinTwoStrings(DATADIR, "ZBufferMaker.glade"), "zIcon", "zbuffermaker")
{
	if (!initSuccess) return;

	theSliderAdjustment = GTK_ADJUSTMENT (gtk_builder_get_object(theXml, "slider_adjustment"));
	theYAdjustment = GTK_ADJUSTMENT (gtk_builder_get_object(theXml, "y_adjustment"));
	theNumBuffersLabel = GTK_LABEL (gtk_builder_get_object(theXml, "num_buffers_label"));
	saveItem = GTK_WIDGET (gtk_builder_get_object(theXml, "save"));
	saveAsItem = GTK_WIDGET (gtk_builder_get_object(theXml, "save_as"));

    init(TRUE);
	setupButtons();
}

// Concrete methods for SludgeApplication:

gboolean SludgeZBufferMaker::init(gboolean calledFromConstructor) 
{
	currentFilename[0] = 0;
	sprintf(currentShortname, "%s", getUntitledFilename());

	setBuffer(1);

	if (calledFromConstructor) {
		backdrop.total=0;
		backdrop.type=2;
		backdrop.sprites=NULL;
		backdrop.myPalette.pal=NULL;
		backdrop.myPalette.r=NULL;
		backdrop.myPalette.g=NULL;
		backdrop.myPalette.b=NULL;

		if (!reserveSpritePal(&backdrop.myPalette, 0)) {
			return TRUE;
		}
	}

    return FALSE;
}

const char * SludgeZBufferMaker::getWindowTitle()
{
	return "SLUDGE Z-Buffer Maker";
}

const char * SludgeZBufferMaker::getFilterName()
{
	return "SLUDGE Z-Buffer Files (*.zbu)";
}

const char * SludgeZBufferMaker::getFilterPattern()
{
	return "*.[zZ][bB][uU]";
}

const char * SludgeZBufferMaker::getUntitledFilename()
{
	return "Untitled Z-Buffer.zbu";
}

gboolean SludgeZBufferMaker::saveFile(char *filename)
{
	return saveZBufferFile(filename, &backdrop);
}

gboolean SludgeZBufferMaker::loadFile(char *filename)
{
	return loadZBufferFile(filename, &backdrop);
}


void SludgeZBufferMaker::postOpen()
{
	gtk_adjustment_set_upper( theSliderAdjustment, backdrop.total-1 );
	gtk_adjustment_set_upper( theYAdjustment, -backdrop.sprites[0].height*2 ); // *2 to allow obscuring characters exiting to the bottom 
	setBuffer(1);
	setBufferY(backdrop.sprites[buffer()].special);
	char buf[5];
	sprintf(buf, "%i", backdrop.total-1);
	gtk_label_set_text(theNumBuffersLabel, buf);

	activateZoomButtons(backdrop.sprites[0].width, -backdrop.sprites[0].height);
	showStatusbar(backdrop.sprites[0].width, -backdrop.sprites[0].height);
	on_zoom_fit_clicked();

	reshape();
	loadZTextures(&backdrop);
	render_timer_event(theDrawingarea);

	setupButtons();
}

void SludgeZBufferMaker::postNew()
{
	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new("Load file to zBuffer",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	setFileChooserFilters(GTK_FILE_CHOOSER (dialog), FALSE, TRUE);

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
		flipBackslashes(&filename);

		if (!loadZBufferFromTGA(filename, &backdrop))
		{
			errorBox("Error", "Loading TGA file failed.");
		} else {
			setFolderFromFilename(filename);
			setFilename(filename);
			int i, lastSlash;
			for (i = 0; filename[i] != 0; i++)
			{
				if (filename[i] == '/')
				lastSlash = i;
			}		
			currentShortname[i-lastSlash-4] = 'z';
			currentShortname[i-lastSlash-3] = 'b';
			currentShortname[i-lastSlash-2] = 'u';
			currentFilename[0] = 0;
			gtk_window_set_title(GTK_WINDOW(theWindow), getWindowTitle());
			setFileChanged();
			postOpen();
		}

		g_free(filename);
	}
	gtk_widget_destroy(dialog);

	setupButtons();
}

// Concrete methods for SludgeGLApplication:

void SludgeZBufferMaker::button1Press(int local_pointx, int local_pointy)
{
	awaitButton1Release = TRUE;
	button1Motion(local_pointx, local_pointy);
}

void SludgeZBufferMaker::button1Release(int local_pointx, int local_pointy)
{
	awaitButton1Release = FALSE;
}

void SludgeZBufferMaker::button1Motion(int local_pointx, int local_pointy)
{
	if (!awaitButton1Release) return;

	int yy = (local_pointy-y)*zmul;

	if ( yy >= 0 && yy <= -backdrop.sprites[0].height*2) {
		setBufferY(yy);
	}
}

void SludgeZBufferMaker::drawingareaLeft() {}

void SludgeZBufferMaker::prepareOpenGL()
{
	if (! backdrop.total) {
		addSprite(0, &backdrop);
		backdrop.sprites[0].width = 640;
		backdrop.sprites[0].height = 480;
	} else
		loadZTextures(&backdrop);

	z = 0.0;
	zmul = (1.0+z/20);
	setCoords();	
}


void SludgeZBufferMaker::drawRect()
{
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
	
	int i;
	int b = buffer();
	
	
	if (backdrop.total>1)
		for (i=1; i< backdrop.total; i++) {
			if (i == b) {
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			} else 
				glColor4f(0.2f, 0.0f, 0.5f, 1.0f);
			
			pasteSprite(&backdrop.sprites[i], &backdrop.myPalette, false);
		}
	
	glDisable(GL_TEXTURE_2D);
	
		
    glColor3f(1.0f, 0.35f, 0.35f);
    glBegin(GL_LINE_LOOP);
    {
        glVertex3f(  0.0,  0.0, 0.0);
        glVertex3f(  backdrop.sprites[0].width,  0.0, 0.0);
        glVertex3f(  backdrop.sprites[0].width, -backdrop.sprites[0].height, 0.0);
        glVertex3f(  0.0, -backdrop.sprites[0].height, 0.0);
    }
    glEnd();

	if (b>0) {
		glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
		glBegin(GL_LINES);
		{
			glVertex3f(  0.0,  backdrop.sprites[b].special, 0.0);
			glVertex3f(  backdrop.sprites[0].width,  backdrop.sprites[b].special, 0.0);
		}
		glEnd();
	}
}


void SludgeZBufferMaker::setupButtons()
{
	if (backdrop.total > 0) {
		gtk_widget_set_sensitive(saveItem, TRUE);
		gtk_widget_set_sensitive(saveAsItem, TRUE);
	} else {
		gtk_widget_set_sensitive(saveItem, FALSE);
		gtk_widget_set_sensitive(saveAsItem, FALSE);
	}
}

struct spriteBank * SludgeZBufferMaker::getBackdrop()
{
	return &backdrop;
}

int SludgeZBufferMaker::bufferY()
{
	return (int)gtk_adjustment_get_value(theYAdjustment);
}

void SludgeZBufferMaker::setBufferY(int i)
{
	gtk_adjustment_set_value(theYAdjustment, (double)i);
}

int SludgeZBufferMaker::buffer()
{
	return (int)gtk_adjustment_get_value(theSliderAdjustment);
}

void SludgeZBufferMaker::setBuffer(int i)
{
	// Validation shouldn't be done here, but I'm cheating.
	if (i < 1) i = 1;
	if (i > backdrop.total-1) i = backdrop.total-1;
	gtk_adjustment_set_value(theSliderAdjustment, (double)i);
}

// Callbacks:

void SludgeZBufferMaker::on_hscale_value_changed(GtkHScale *theScale)
{
//	if (buffer() > 0)
//		[bufferYTextField setEnabled:YES];
//	else
//		[bufferYTextField setEnabled:NO];
	setBufferY(backdrop.sprites[buffer()].special);
	render_timer_event(theDrawingarea);
}

void SludgeZBufferMaker::on_spinbutton_value_changed(GtkSpinButton *theSpinButton)
{
	if (buffer() && bufferY() != backdrop.sprites[buffer()].special) {
		backdrop.sprites[buffer()].special = bufferY();
		setFileChanged();
	}
	render_timer_event(theDrawingarea);
}
