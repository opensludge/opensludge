/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeSpriteBankEditor.cpp - Part of the SLUDGE Sprite Bank Editor (GTK+ version)
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
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "Common.h"

#include "SludgeSpriteBankEditor.h"

SludgeSpriteBankEditor::SludgeSpriteBankEditor()
 : SludgeGLApplication(joinTwoStrings(DATADIR, "SpriteBankEditor.glade"), "spriteIcon", "spritebankeditor")
{
	if (!initSuccess) return;

	ignoreModePalButtons = FALSE;

	theSliderAdjustment = GTK_ADJUSTMENT (gtk_builder_get_object(theXml, "slider_adjustment"));
	theXAdjustment = GTK_ADJUSTMENT (gtk_builder_get_object(theXml, "x_adjustment"));
	theYAdjustment = GTK_ADJUSTMENT (gtk_builder_get_object(theXml, "y_adjustment"));
	theNumSpritesLabel = GTK_LABEL (gtk_builder_get_object(theXml, "num_sprites_label"));
	modePalButton[0] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "mode_pal_open"));
	modePalButton[1] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "mode_pal_closed"));
	modePalButton[2] = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "mode_pal_none"));
	insertButton = GTK_WIDGET (gtk_builder_get_object(theXml, "insert_sprite"));
	deleteButton = GTK_WIDGET (gtk_builder_get_object(theXml, "delete_sprite"));
	exportButton = GTK_WIDGET (gtk_builder_get_object(theXml, "export_sprite"));
	replaceButton = GTK_WIDGET (gtk_builder_get_object(theXml, "replace_sprite"));
	centreButton = GTK_WIDGET (gtk_builder_get_object(theXml, "centre_hotspot"));
	baseButton = GTK_WIDGET (gtk_builder_get_object(theXml, "base_hotspot"));
	showBoxButton = GTK_WIDGET (gtk_builder_get_object(theXml, "show_box"));
	xSpinButton = GTK_WIDGET (gtk_builder_get_object(theXml, "x_spinbutton"));
	ySpinButton = GTK_WIDGET (gtk_builder_get_object(theXml, "y_spinbutton"));

    init();
}

SludgeSpriteBankEditor::~SludgeSpriteBankEditor()
{
	if (sprites.total) {
		forgetSpriteBank(&sprites);
		sprites.total = 0;
	}
}

// Concrete methods for SludgeApplication:

gboolean SludgeSpriteBankEditor::init() 
{
	sprites.total=0;
	if (!reserveSpritePal(&sprites.myPalette, 0)) {
			return TRUE;
	}

	setupButtons();

    return FALSE;
}

const char * SludgeSpriteBankEditor::getWindowTitle()
{
	return "SLUDGE Sprite Bank Editor";
}

const char * SludgeSpriteBankEditor::getFilterName()
{
	return "SLUDGE Sprite Bank Files (*.duc)";
}

const char * SludgeSpriteBankEditor::getFilterPattern()
{
	return "*.[dD][uU][cC]";
}

const char * SludgeSpriteBankEditor::getUntitledFilename()
{
	return "Untitled Sprite Bank.duc";
}

gboolean SludgeSpriteBankEditor::saveFile(char *filename)
{
	return saveSpriteBank(filename, &sprites);
}

gboolean SludgeSpriteBankEditor::loadFile(char *filename)
{
	return loadSpriteBank(filename, &sprites);
}

void SludgeSpriteBankEditor::postOpen()
{
	on_hscale_value_changed();
	setupButtons();
	setupPalButtons();
	prepareOpenGL();
	on_zoom_fit_clicked();
	render_timer_event(theDrawingarea);
}

void SludgeSpriteBankEditor::postNew()
{
	if (sprites.total) {
		forgetSpriteBank(&sprites);
		sprites.total = 0;
	}

	init();

	setHotSpotX(0);
	setHotSpotY(0);
	setSpriteIndex(0);
	setupButtons();
	setupPalButtons();
}

// Concrete methods for SludgeGLApplication:

void SludgeSpriteBankEditor::button1Press(int local_pointx, int local_pointy)
{
	awaitButton1Release = TRUE;

	hotSpotX1 = hotSpotX();
	hotSpotY1 = hotSpotY();
	mouseLoc1x = local_pointx;
	mouseLoc1y = local_pointy;
}

void SludgeSpriteBankEditor::button1Release(int local_pointx, int local_pointy)
{
	awaitButton1Release = FALSE;
}

void SludgeSpriteBankEditor::button1Motion(int local_pointx, int local_pointy)
{
	if (!awaitButton1Release) return;

	setHotSpotX(hotSpotX1 + zmul * (mouseLoc1x - local_pointx));
	setHotSpotY(hotSpotY1 + zmul * (mouseLoc1y - local_pointy));
}

void SludgeSpriteBankEditor::drawingareaLeft() {}

void SludgeSpriteBankEditor::prepareOpenGL()
{
	showBox = false;
	if (sprites.total)
		loadSpriteTextures(&sprites);
	setSpriteIndex(0);
}

void SludgeSpriteBankEditor::drawRect()
{
	setCoords();
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 0.35f, 0.35f);
	glBegin(GL_LINES);
    {
		glVertex3f(  -1000.0,  0.0, 0.0);
		glVertex3f(  1000.0,  0.0, 0.0);
		glVertex3f(  0.0, -1000.0, 0.0);
		glVertex3f(  0.0, 1000.0, 0.0);
    }
    glEnd();
	
    glColor3f(1.0f, 1.0f, 1.0f);
	
	if (spriteIndex() < sprites.total)
		pasteSprite(&sprites.sprites[spriteIndex()], &sprites.myPalette, showBox);
}


void SludgeSpriteBankEditor::setCoords() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x*zmul, (x+w)*zmul, (y-h)*zmul, y*zmul, 1.0, -1.0);	
	glMatrixMode(GL_MODELVIEW);	
}


void SludgeSpriteBankEditor::setupButtons()
{
	if (sprites.total < 256) {
		gtk_adjustment_set_upper(theSliderAdjustment, sprites.total);
		char buf[5];
		sprintf(buf, "%i", sprites.total);
		gtk_label_set_text(theNumSpritesLabel, buf);
		gtk_widget_set_sensitive(insertButton, TRUE);
	} else {
		gtk_adjustment_set_upper(theSliderAdjustment, 255.);
		gtk_label_set_text(theNumSpritesLabel, "255");
		gtk_widget_set_sensitive(insertButton, FALSE);
	}

	if (spriteIndex() < sprites.total && sprites.total) {
		gtk_widget_set_sensitive(deleteButton, TRUE);
		gtk_widget_set_sensitive(exportButton, TRUE);
		gtk_widget_set_sensitive(replaceButton, TRUE);
		gtk_widget_set_sensitive(centreButton, TRUE);
		gtk_widget_set_sensitive(baseButton, TRUE);
		gtk_widget_set_sensitive(showBoxButton, TRUE);
		gtk_widget_set_sensitive(xSpinButton, TRUE);
		gtk_widget_set_sensitive(ySpinButton, TRUE);
	} else {
		gtk_widget_set_sensitive(deleteButton, FALSE);
		gtk_widget_set_sensitive(exportButton, FALSE);
		gtk_widget_set_sensitive(replaceButton, FALSE);
		gtk_widget_set_sensitive(centreButton, FALSE);
		gtk_widget_set_sensitive(baseButton, FALSE);
		gtk_widget_set_sensitive(showBoxButton, FALSE);
		gtk_widget_set_sensitive(xSpinButton, FALSE);
		gtk_widget_set_sensitive(ySpinButton, FALSE);
	}

	if (spriteIndex() >= sprites.total) {
		deactivateZoomButtons();
		showStatusbar(0, 0);
	} else if (sprites.total) {
		activateZoomButtons(sprites.sprites[spriteIndex()].width, sprites.sprites[spriteIndex()].height);
		showStatusbar(sprites.sprites[spriteIndex()].width, sprites.sprites[spriteIndex()].height);
	} else {
		deactivateZoomButtons();
		hideStatusbar();
	}
}

void SludgeSpriteBankEditor::setupPalButtons()
{
	if (sprites.total) {
		if (sprites.type < 2) {
			sprites.type = 1;
			ignoreModePalButtons = TRUE;
			gtk_toggle_button_set_active(modePalButton[1], TRUE);
			ignoreModePalButtons = FALSE;
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), TRUE);
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), TRUE);
		} else {
			ignoreModePalButtons = TRUE;
			gtk_toggle_button_set_active(modePalButton[2], TRUE);
			ignoreModePalButtons = FALSE;
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), FALSE);
		}
	} else {
		ignoreModePalButtons = TRUE;
		gtk_toggle_button_set_active(modePalButton[0], TRUE);
		ignoreModePalButtons = FALSE;
		gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), FALSE);
	}
}

int SludgeSpriteBankEditor::hotSpotX()
{
	return (int)gtk_adjustment_get_value(theXAdjustment);
}

void SludgeSpriteBankEditor::setHotSpotX(int i)
{
	gtk_adjustment_set_value(theXAdjustment, (double)i);
}

int SludgeSpriteBankEditor::hotSpotY()
{
	return (int)gtk_adjustment_get_value(theYAdjustment);
}

void SludgeSpriteBankEditor::setHotSpotY(int i)
{
	gtk_adjustment_set_value(theYAdjustment, (double)i);
}

int SludgeSpriteBankEditor::spriteIndex()
{
	return (int)gtk_adjustment_get_value(theSliderAdjustment);
}

void SludgeSpriteBankEditor::setSpriteIndex(int i)
{
	// Validation shouldn't be done here, but I'm cheating.
	if (i > sprites.total) i = sprites.total;
	if (i > 255) i = 255;
	else if (i<0) i = 0;
	gtk_adjustment_set_value(theSliderAdjustment, (double)i);
}

void SludgeSpriteBankEditor::centreHotSpot(int index)
{
	if (sprites.total) {
		sprites.sprites[index].xhot = sprites.sprites[index].width / 2;
		sprites.sprites[index].yhot = sprites.sprites[index].height / 2;
		if (index == spriteIndex()) {
			setHotSpotX(sprites.sprites[index].width / 2);
			setHotSpotY(sprites.sprites[index].height / 2);
		}
	}
}

void SludgeSpriteBankEditor::baseHotSpot(int index)
{
	if (sprites.total) {
		sprites.sprites[index].xhot = sprites.sprites[index].width / 2;
		sprites.sprites[index].yhot = sprites.sprites[index].height-1;
		if (index == spriteIndex()) {
			setHotSpotX(sprites.sprites[index].width / 2);
			setHotSpotY(sprites.sprites[index].height-1);
		}
	}
}

int SludgeSpriteBankEditor::loadSprites(int toIndex, gboolean addNew)
{
	gboolean success;
	int spritesLoaded = 0;
	GtkWidget *dialog;
	GtkFileFilter *pngtgafilter, *pngfilter, *tgafilter;

	dialog = gtk_file_chooser_dialog_new("Load file as sprite",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	pngtgafilter = gtk_file_filter_new();
	gtk_file_filter_set_name(pngtgafilter, "PNG/TGA images");
	gtk_file_filter_add_mime_type(pngtgafilter, "image/png");
	gtk_file_filter_add_mime_type(pngtgafilter, "image/x-tga");
	gtk_file_filter_add_pattern(pngtgafilter, "*.[tT][gG][aA]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), pngtgafilter);

	pngfilter = gtk_file_filter_new();
	gtk_file_filter_set_name(pngfilter, "PNG images");
	gtk_file_filter_add_mime_type(pngfilter, "image/png");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), pngfilter);

	tgafilter = gtk_file_filter_new();
	gtk_file_filter_set_name(tgafilter, "TGA images");
	gtk_file_filter_add_mime_type(tgafilter, "image/x-tga");
	gtk_file_filter_add_pattern(tgafilter, "*.[tT][gG][aA]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), tgafilter);

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER (dialog), pngtgafilter);

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER (dialog), TRUE);

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *filenames;
		char *filename;
		filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER (dialog));

		for (int j = 0; j < g_slist_length(filenames); j++) {

			success = FALSE;

			if (addNew || toIndex + spritesLoaded >= sprites.total)
				addSprite(toIndex + spritesLoaded, &sprites);

			filename = (char *) g_slist_nth(filenames, j)->data;
			flipBackslashes(&filename);

			if (strlen(filename) > 4) {
				char * extension = filename + strlen(filename) - 4;
				if        (!strcmp(extension, ".png") || !strcmp(extension, ".PNG")) {
					success = loadSpriteFromPNG(filename, &sprites, toIndex + spritesLoaded);
				} else if (!strcmp(extension, ".tga") || !strcmp(extension, ".TGA")) {
					success = loadSpriteFromTGA(filename, &sprites, toIndex + spritesLoaded);
				}
			} else {
				errorBox("Can't load image", "I don't recognise the file type. TGA and PNG are the supported file types.");
			}
		
			if (g_slist_nth(filenames, j) != g_slist_last(filenames)) {
				g_free(filename);
			}

			if (success) {
				baseHotSpot(toIndex + spritesLoaded);
				spritesLoaded++;
				setFileChanged();
			} else if (addNew || toIndex + spritesLoaded >= sprites.total) {
				deleteSprite(toIndex + spritesLoaded, &sprites);
			}
		}

		if (spritesLoaded > 0) {
			setFolderFromFilename(filename);
			render_timer_event(theDrawingarea);
		}

		g_free(filename);
		g_slist_free(filenames);
	}
	gtk_widget_destroy(dialog);

	return spritesLoaded;
}

// Callbacks:

void SludgeSpriteBankEditor::on_show_box_toggled(GtkToggleButton *theButton)
{
	showBox = gtk_toggle_button_get_active(theButton);
	render_timer_event(theDrawingarea);
}

void SludgeSpriteBankEditor::on_x_spinbutton_value_changed()
{
	if ((spriteIndex() < sprites.total) && (hotSpotX() != sprites.sprites[spriteIndex()].xhot)) {
		sprites.sprites[spriteIndex()].xhot = hotSpotX();
		setFileChanged();
		render_timer_event(theDrawingarea);
	}
}

void SludgeSpriteBankEditor::on_y_spinbutton_value_changed()
{
	if ((spriteIndex() < sprites.total) && (hotSpotY() != sprites.sprites[spriteIndex()].yhot)) {
		sprites.sprites[spriteIndex()].yhot = hotSpotY();
		setFileChanged();
		render_timer_event(theDrawingarea);
	}
}

void SludgeSpriteBankEditor::on_centre_hotspot_clicked()
{
	centreHotSpot(spriteIndex());
}

void SludgeSpriteBankEditor::on_base_hotspot_clicked()
{
	baseHotSpot(spriteIndex());
}

void SludgeSpriteBankEditor::on_mode_pal_clicked(int mode)
{
	if (ignoreModePalButtons) return;
	if (mode == 2)
	{
		if (sprites.total && sprites.type < 2)
		{
			if (!askAQuestion("Convert sprite bank?", "This will convert your entire sprite bank to 32-bit colour mode. This action can not be reversed. Do you want to proceed?"))
			{
				ignoreModePalButtons = TRUE;
				gtk_toggle_button_set_active(modePalButton[sprites.type], TRUE);
				ignoreModePalButtons = FALSE;
				return;
			}
			if (!convertSpriteBank8to32(&sprites))
			{
				ignoreModePalButtons = TRUE;
				gtk_toggle_button_set_active(modePalButton[sprites.type], TRUE);
				ignoreModePalButtons = FALSE;
				return;
			}
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), FALSE);
		}
	}

	sprites.type = mode;
	setFileChanged();
}

void SludgeSpriteBankEditor::on_insert_sprite_clicked(gboolean atTheEnd, gboolean addNew)
{
	int position, spritesLoaded, totalAtStart;

	totalAtStart = sprites.total;

	if (atTheEnd) {
		position = sprites.total;
	} else {
		position = spriteIndex();
	}

	spritesLoaded = loadSprites(spriteIndex(), addNew);
	if (spritesLoaded) {
		setupButtons();
		setSpriteIndex(position + spritesLoaded - 1);
		if (!totalAtStart) on_zoom_fit_clicked();
		if (sprites.type < 2) {
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), TRUE);
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), TRUE);
		} else {
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[0]), FALSE);
			gtk_widget_set_sensitive(GTK_WIDGET(modePalButton[1]), FALSE);
		}
	}
}

void SludgeSpriteBankEditor::on_delete_sprite_clicked()
{
	if (! askAQuestion("Delete this sprite?", "Really delete this sprite?") )
		return;
	deleteSprite(spriteIndex(), &sprites);
	setFileChanged();
	if (spriteIndex() >= sprites.total) {
		setSpriteIndex(sprites.total-1);
	} else {
		on_hscale_value_changed();
	}
	if (!sprites.total) {
		postNew();
	}
}

void SludgeSpriteBankEditor::on_export_sprite_clicked()
{
	GtkWidget *dialog;
	GtkFileFilter *filter;

	dialog = gtk_file_chooser_dialog_new("Export sprite",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "PNG images");
	gtk_file_filter_add_mime_type(filter, "image/png");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER (dialog), filter);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), "Untitled Sprite.png");

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
		flipBackslashes(&filename);

		exportToPNG(filename, &sprites, spriteIndex());

		setFolderFromFilename(filename);

		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void SludgeSpriteBankEditor::on_hscale_value_changed()
{
	if (spriteIndex() < sprites.total) {
		setHotSpotX(sprites.sprites[spriteIndex()].xhot);
		setHotSpotY(sprites.sprites[spriteIndex()].yhot);
	} else {
		setHotSpotX(0);
		setHotSpotY(0);
	}
	setupButtons();
	render_timer_event(theDrawingarea);
}

void SludgeSpriteBankEditor::on_zoom_100_clicked()
{
	setZ(0);
	x = -w/2;
	y = h*3/4;
	reshape();
}

void SludgeSpriteBankEditor::on_zoom_fit_clicked()
{
	float zmulx, zmuly;
	zmulx = (float) picWidth / w * 2.;
	zmuly = (float) picHeight / h * 2.;

	if (zmulx > zmuly) {
		zmul = zmulx;
	} else {
		zmul = zmuly;
	}
	z =	20. * (zmul - 1.);

	x = -w/2;
	y = h*3/4;

	refreshStatusbarZmul();
	reshape();
}
