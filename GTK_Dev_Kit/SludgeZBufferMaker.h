/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeZBufferMaker.h - Part of the SLUDGE Z-Buffer Maker (GTK+ version)
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

class SludgeZBufferMaker : public SludgeGLApplication {

private:
	struct spriteBank backdrop;

	GtkAdjustment *theSliderAdjustment;
	GtkAdjustment *theYAdjustment;
	GtkLabel *theNumBuffersLabel;

	GtkWidget *saveItem, *saveAsItem;

	int mouseLoc1x, mouseLoc1y;


private:
	// Concrete methods for SludgeApplication:
	virtual gboolean init(gboolean calledFromConstructor);
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

	void setupButtons();
	struct spriteBank * getBackdrop();
	int bufferY();
	void setBufferY(int i);
	int buffer();
	void setBuffer(int i);

public:
	SludgeZBufferMaker();
	// Callbacks:
	void on_hscale_value_changed(GtkHScale *theScale);
	void on_spinbutton_value_changed(GtkSpinButton *theSpinButton);
};
