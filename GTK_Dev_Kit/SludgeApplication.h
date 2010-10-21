/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeApplication.h - Part of the SLUDGE Dev Kit (GTK+ version)
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

class SludgeApplication {

private:
	const char * configfile;
	gboolean fileChanged;

protected:
	char currentFilename[260], currentShortname[260], currentFolder[260];

public:
	GtkWidget *theWindow;
	GtkBuilder *theXml;


private:
	virtual gboolean init() = 0;
	virtual const char * getWindowTitle() = 0;
	virtual const char * getFilterName() = 0;
	virtual const char * getFilterPattern() = 0;
	virtual const char * getUntitledFilename() = 0;
	virtual gboolean saveFile(char *filename) = 0;
	virtual gboolean loadFile(char *filename) = 0;
	virtual void postNew() = 0;
	virtual void postOpen() = 0;

	void saveToFile(gboolean saveAs);
	gboolean reallyClose();

protected:
	void setFilename(char* filename);
	void setFolderFromFilename(char* filename);
	void setFileChanged();

public:
	SludgeApplication(const char * gladeFileName, const char * iconName, const char * configFile);
	~SludgeApplication();
	void open(char* filename);
	// Callbacks:
	gboolean on_window1_delete_event();
	void on_new();
	void on_open();
	void on_save();
	void on_save_as();
	void on_about();
};
