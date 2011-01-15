/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeSpriteBankEditor.h - Part of the SLUDGE Sprite Bank Editor (GTK+ version)
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

#include "SludgeGLApplication.h"
#include "sprites.h"

class SludgeSpriteBankEditor : public SludgeGLApplication {

private:
	struct spriteBank sprites;
	bool showBox;
	int fontifySpaceWidth;

	GtkAdjustment *theSliderAdjustment;
	GtkAdjustment *theXAdjustment, *theYAdjustment;
	GtkLabel *theNumSpritesLabel;
	GtkToggleButton *modePalButton[3];
	GtkWidget *insertButton, *deleteButton, *replaceButton, *exportButton, *centreButton, *baseButton, *multiButton;
	GtkWidget *showBoxButton, *xSpinButton, *ySpinButton;
	GtkDialog *fontifyDialog, *multiDialog;
	GtkSpinButton *fontifySpinButton, *multiFromSpinButton, *multiToSpinButton;
	GtkAdjustment *multiFromAdjustment, *multiToAdjustment;

	int hotSpotX1, hotSpotY1, mouseLoc1x, mouseLoc1y;
	gboolean ignoreModePalButtons;


private:
	// Concrete methods for SludgeApplication:
	virtual gboolean init();
	virtual const char * getWindowTitle();
	virtual const char * getFilterName();
	virtual const char * getFilterPattern();
	virtual const char * getUntitledFilename();
	virtual gboolean saveFile(char *filename);
	virtual gboolean loadFile(char *filename);
	virtual void postNew();
	virtual void postOpen();

	// Concrete methods for SludgeGLApplication:
	virtual void button1Press(int local_pointx, int local_pointy);
	virtual void button1Release(int local_pointx, int local_pointy);
	virtual void button1Motion(int local_pointx, int local_pointy);
	virtual void drawingareaLeft();
	virtual void prepareOpenGL();
	virtual void drawRect();

	void setCoords();

	void setupButtons();
	void setupPalButtons();
	int hotSpotX();
	void setHotSpotX(int i);
	int hotSpotY();
	void setHotSpotY(int i);
	int spriteIndex();
	void setSpriteIndex(int i);
	void centreHotSpot(int index);
	void baseHotSpot(int index);
	int loadSprites(int toIndex, gboolean addNew);

public:
	SludgeSpriteBankEditor();
	~SludgeSpriteBankEditor();
	// Callbacks:
	void on_show_box_toggled(GtkToggleButton *theButton);
	void on_x_spinbutton_value_changed();
	void on_y_spinbutton_value_changed();
	void on_centre_hotspot_clicked();
	void on_base_hotspot_clicked();
	void on_multi_hotspot_clicked();
	void on_mode_pal_clicked(int mode);
	void on_insert_sprite_clicked(gboolean atTheEnd, gboolean addNew);
	void on_delete_sprite_clicked();
	void on_export_sprite_clicked();
	void on_hscale_value_changed();
	void on_fontify();

	void on_zoom_100_clicked();
	void on_zoom_fit_clicked();
};
