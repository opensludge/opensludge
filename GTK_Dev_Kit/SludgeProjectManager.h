/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeProjectManager.h - Part of the SLUDGE Project Manager (GTK+ version)
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

enum whichTreeview {
	FILE_TREEVIEW,
	RESOURCE_TREEVIEW,
	ERROR_TREEVIEW
};

enum whichProgram {
	FLOORMAKER,
	SPRITEBANKEDITOR,
	ZBUFFERMAKER,
	TRANSLATIONEDITOR
};

class SludgeProjectManager : public SludgeApplication {

private:
	char workingDir[1000];
	char editor[1000], imageViewer[1000], audioPlayer[1000], modPlayer[1000];

	int numResources;
	char *resourceList[1000];
	char *fileList[1000];
	int fileListNum;

	GtkListStore *filesListStore;
	GtkListStore *resourcesListStore;
	GtkListStore *errorsListStore;
	GtkTreeSelection *filesSelection;

	GtkNotebook *notebook;

	GtkWidget *saveItem, *saveAsItem, *projectPropertiesItem, *projectCompileItem, *projectRunGameItem;
	GtkWidget *addFileButton, *removeFileButton;

	GtkProgressBar *compProgress1, *compProgress2;
	GtkLabel *compTask, *compFile, *compItem, *compFuncs;
	GtkLabel *compObjs, *compGlobs, *compStrings, *compResources;
	GtkWidget *runGameButton, *closeCompilerButton;

public:
	GAsyncQueue *compilerInfoQueue;


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

	void setupButtons();
	void listChanged(whichTreeview whichOne);
	void readIniFile();
	void saveIniFile();

public:
	SludgeProjectManager();
	~SludgeProjectManager();
	void compile();
	void update_compile_window();
	// Callbacks:
	void on_treeview_realize(GtkTreeView *theTreeView, whichTreeview whichOne);
	void on_files_tree_selection_changed(GtkTreeSelection *theSelection);
	void on_treeview_row_activated(GtkTreeView *theTreeView, GtkTreePath *thePath, GtkTreeViewColumn *theColumn, whichTreeview whichOne);
	void on_add_file_clicked();
	void on_remove_file_clicked();
	void on_compile();
	void on_comp_okbutton_clicked(GtkButton *theButton);
	void on_comp_gamebutton_clicked();
	void on_project_settings();
	void on_preferences();
	void on_program_activate(whichProgram program);
};
