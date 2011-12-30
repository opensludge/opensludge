/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeTranslationEditor.h - Part of the SLUDGE Translation Editor (GTK+ version)
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

enum {
  COLUMN_ORIGINAL,
  COLUMN_TRANSLATE,
  COLUMN_TRANSLATION,
  COLUMN_ID,
  COLUMN_VISIBLE,
  N_COLUMNS
};

class SludgeTranslationEditor : public SludgeApplication {

private:
	struct transLine * firstTransLine;
	char * langName;

	gboolean badLangName;

	GtkComboBox *comboBox;
	GtkListStore *listStore;
	GtkTreeModel *filterModel, *sortModel;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *originalColumn, *translationColumn;
	GtkAdjustment *theIdAdjustment;
	GtkEntry *theLanguageEntry;
	GtkEntry *theSearchEntry;
	GtkTextBuffer *theOriginalTextBuffer, *theTranslationTextBuffer;


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

	void listChanged();

public:
	SludgeTranslationEditor();
	// Callbacks:
	void on_combobox_realize(GtkComboBox *theComboBox);
	void on_combobox_changed(GtkComboBox *theComboBox);
	void on_treeview_realize(GtkTreeView *theTreeView);
	void on_tree_selection_changed(GtkTreeSelection *theSelection);
	void on_column_changed(int column, GtkCellRenderer *theCell_renderer, gchar *thePath, gchar *theNewText);
	void on_sort_clicked(GtkTreeViewColumn *theTreeViewColumn, int sortColumn);
	void on_load_strings_clicked();
	gboolean searchEqualFunc(GtkTreeModel *model, const gchar *key, GtkTreeIter *iter);
};
