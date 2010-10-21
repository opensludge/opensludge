/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeFloorMaker.h - Part of the SLUDGE Floor Maker (GTK+ version)
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

class SludgeFloorMaker : public SludgeGLApplication {

private:
	float r,g,b;
	int lit, litX, litY;
	int selection, selx1, sely1, selx2, sely2;
	int mouseLoc1x, mouseLoc1y;
	struct spriteBank backdrop;
	struct polyList * firstPoly;
	int mode;


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

	struct polyList * getFloor();
	void setFloor(struct polyList * floor);

public:
	SludgeFloorMaker();
	~SludgeFloorMaker();
	// Callbacks:
	void on_filechooserbutton_realize(GtkFileChooser *theChooser);
	void on_filechooserbutton_file_set(GtkFileChooser *theChooser);
	void on_colorbutton_realize(GtkColorButton *theButton);
	void on_colorbutton_color_set(GtkColorButton *theButton);
	void on_mode_changed(int buttonClicked);
};
