/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * SludgeProjectManager.cpp - Part of the SLUDGE Project Manager (GTK+ version)
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
#include <glib/gstdio.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include "project.hpp"
#include "settings.h"
#include "interface.h"
#include "compiler.hpp"
#include "moreio.h"
#include "helpers.h"
#include "errorlinktofile.h"
#include "Common.h"

#include "SludgeProjectManager.h"
#include "ProjectManagerMain.h"

extern char * gameFile;

SludgeProjectManager::SludgeProjectManager()
 : SludgeApplication(joinTwoStrings(DATADIR, "ProjectManager.glade"), "ProjIcon", "projectmanager")
{
	if (!initSuccess) return;

	numResources = 0;
	resourceList[0] = NULL;
	fileList[0] = NULL;

	filesListStore = NULL;
	resourcesListStore = NULL;
	errorsListStore = NULL;
	filesSelection = NULL;

	notebook = GTK_NOTEBOOK (gtk_builder_get_object(theXml, "notebook"));
	saveItem = GTK_WIDGET (gtk_builder_get_object(theXml, "save"));
	saveAsItem = GTK_WIDGET (gtk_builder_get_object(theXml, "save_as"));
	projectPropertiesItem = GTK_WIDGET (gtk_builder_get_object(theXml, "project_properties"));
	projectCompileItem = GTK_WIDGET (gtk_builder_get_object(theXml, "project_compile"));
	projectRunGameItem = GTK_WIDGET (gtk_builder_get_object(theXml, "project_run_game"));
	addFileButton = GTK_WIDGET (gtk_builder_get_object(theXml, "add_file"));
	removeFileButton = GTK_WIDGET (gtk_builder_get_object(theXml, "remove_file"));

	compilerDialog = GTK_DIALOG (gtk_builder_get_object(theXml, "compiler_dialog"));
	compProgress1 = GTK_PROGRESS_BAR (gtk_builder_get_object(theXml, "progressbar1"));
	compProgress2 = GTK_PROGRESS_BAR (gtk_builder_get_object(theXml, "progressbar2"));
	compTask = GTK_LABEL (gtk_builder_get_object(theXml, "comp_task"));
	compFile = GTK_LABEL (gtk_builder_get_object(theXml, "comp_file"));
	compItem = GTK_LABEL (gtk_builder_get_object(theXml, "comp_item"));
	compFuncs = GTK_LABEL (gtk_builder_get_object(theXml, "comp_funcs"));
	compObjs = GTK_LABEL (gtk_builder_get_object(theXml, "comp_objs"));
	compGlobs = GTK_LABEL (gtk_builder_get_object(theXml, "comp_globs"));
	compStrings = GTK_LABEL (gtk_builder_get_object(theXml, "comp_strings"));
	compResources = GTK_LABEL (gtk_builder_get_object(theXml, "comp_resources"));
	runGameButton = GTK_WIDGET (gtk_builder_get_object(theXml, "comp_gamebutton"));
	closeCompilerButton = GTK_WIDGET (gtk_builder_get_object(theXml, "comp_okbutton"));

	projectSettingsDialog = GTK_DIALOG (gtk_builder_get_object(theXml, "project_settings_dialog"));
	prefName = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_name"));
	prefQuit = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_quit"));
	prefSave = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_save"));
	prefLanguage = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_language"));
	prefFilename = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_filename"));
	prefIcon = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_icon"));
	prefLogo = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_logo"));
	prefWidth = GTK_SPIN_BUTTON (gtk_builder_get_object(theXml, "pref_width"));
	prefHeight = GTK_SPIN_BUTTON (gtk_builder_get_object(theXml, "pref_height"));
	prefSpeed = GTK_SPIN_BUTTON (gtk_builder_get_object(theXml, "pref_speed"));
	prefSilent = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "pref_silent"));

	preferenceDialog = GTK_DIALOG (gtk_builder_get_object(theXml, "preferences_dialog"));
	prefKeepImages = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "pref_keep_images"));
	prefWriteStrings = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "pref_write_strings"));
	prefVerbose = GTK_TOGGLE_BUTTON (gtk_builder_get_object(theXml, "pref_verbose"));
	prefEditor = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_editor"));
	prefImageViewer = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_image_viewer"));
	prefAudioPlayer = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_audio_player"));
	prefModPlayer = GTK_ENTRY (gtk_builder_get_object(theXml, "pref_mod_player"));

	compilerInfoQueue = NULL;

	if (!getcwd(workingDir, 998))
		fprintf(stderr, "Couldn't get current working directory.");

	readIniFile();

    init(TRUE);

	setupButtons();
}

SludgeProjectManager::~SludgeProjectManager()
{
	closeProject(fileList, &fileListNum);
}

// Concrete methods for SludgeApplication:

gboolean SludgeProjectManager::init(gboolean calledFromConstructor) 
{
	currentFilename[0] = 0;
	sprintf(currentShortname, "%s", getUntitledFilename());

    return FALSE;
}

const char * SludgeProjectManager::getWindowTitle()
{
	return "SLUDGE Project Manager";
}

const char * SludgeProjectManager::getFilterName()
{
	return "SLUDGE Project Files (*.slp)";
}

const char * SludgeProjectManager::getFilterPattern()
{
	return "*.[sS][lL][pP]";
}

const char * SludgeProjectManager::getUntitledFilename()
{
	return "Untitled Project.slp";
}

gboolean SludgeProjectManager::saveFile(char *filename)
{
	return saveProject(filename, fileList, &fileListNum);
}

gboolean SludgeProjectManager::loadFile(char *filename)
{
	return loadProject(filename, fileList, &fileListNum);
}


void SludgeProjectManager::postOpen()
{
	listChanged(FILE_TREEVIEW);
	setupButtons();
	gtk_widget_set_sensitive(projectRunGameItem, FALSE);
}

void SludgeProjectManager::postNew()
{
	GtkWidget *dialog;
	GtkFileFilter *filter;
	gboolean success = FALSE;

	dialog = gtk_file_chooser_dialog_new("New SLUDGE Project",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, getFilterName());
	gtk_file_filter_add_pattern(filter, getFilterPattern());
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER (dialog), filter);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), getUntitledFilename());

	if (currentFolder[0] != 0)
	{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (dialog), currentFolder);
	}

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog));
		flipBackslashes(&filename);

		doNewProject(filename, fileList, &fileListNum);
		listChanged(FILE_TREEVIEW);
		listChanged(RESOURCE_TREEVIEW);
		gtk_list_store_clear(errorsListStore);
		gtk_widget_set_sensitive(projectRunGameItem, FALSE);

		setFilename(filename);
		setFolderFromFilename(filename);

		g_free(filename);
		success = TRUE;
	}
	gtk_widget_destroy(dialog);

	if (success)
		on_project_settings();

	setupButtons();
}

void SludgeProjectManager::setupButtons()
{
	if (currentFilename[0] == 0) {
		gtk_widget_set_sensitive(saveItem, FALSE);
		gtk_widget_set_sensitive(saveAsItem, FALSE);
		gtk_widget_set_sensitive(projectPropertiesItem, FALSE);
		gtk_widget_set_sensitive(addFileButton, FALSE);
		gtk_widget_set_sensitive(removeFileButton, FALSE);
	} else {
		gtk_widget_set_sensitive(saveItem, TRUE);
		gtk_widget_set_sensitive(saveAsItem, TRUE);
		gtk_widget_set_sensitive(projectPropertiesItem, TRUE);
		gtk_widget_set_sensitive(addFileButton, TRUE);
	}

	if (fileListNum) {
		gtk_widget_set_sensitive(projectCompileItem, TRUE);
	} else {
		gtk_widget_set_sensitive(projectCompileItem, FALSE);
	}
}

void SludgeProjectManager::listChanged(whichTreeview whichOne)
{
	GtkListStore *listStore;
	int numItems;
	char **list;

	switch (whichOne) {
		case FILE_TREEVIEW:
			listStore = filesListStore;
			numItems = fileListNum;
			list = fileList;
			break;
		case RESOURCE_TREEVIEW:
			listStore = resourcesListStore;
			numItems = numResources;
			list = resourceList;
			break;
		case ERROR_TREEVIEW:
			listStore = errorsListStore;
			numItems = numErrors;
			break;
		default:
			break;
	}

	gtk_list_store_clear(listStore);

	int i, j;
	char *listitem;
	struct errorLinkToFile * index;
	for (i = 0; i < numItems; i++) {
		GtkTreeIter iter;
		gtk_list_store_append(listStore, &iter);
		switch (whichOne) {
			case FILE_TREEVIEW:
			case RESOURCE_TREEVIEW:
			{
				listitem = g_filename_to_utf8(list[i], -1, NULL, NULL, NULL);
				gtk_list_store_set(listStore, &iter, 0, listitem, -1);
				g_free(listitem);
				listitem = NULL;
				break;
			}
			case ERROR_TREEVIEW:
			{
				index = errorList;
				if (! index) return;
				j = numErrors-1;
				while (j>i) {
					if (! (index = index->next)) return;
					j--;
				}
				listitem = g_locale_to_utf8(index->fullText, -1, NULL, NULL, NULL);
				gtk_list_store_set(listStore, &iter, ERRORS_COLUMN_FULLTEXT, listitem, -1);
				g_free(listitem);
				listitem = NULL;
				if (index->filename) {
					gtk_list_store_set(listStore, &iter, ERRORS_COLUMN_HAS_FILENAME, TRUE, -1);

					listitem = g_filename_to_utf8(index->filename, -1, NULL, NULL, NULL);
					gtk_list_store_set(listStore, &iter, ERRORS_COLUMN_FILENAME, listitem, -1);
					g_free(listitem);
					listitem = NULL;
				} else {
					gtk_list_store_set(listStore, &iter, ERRORS_COLUMN_HAS_FILENAME, FALSE, -1);

					listitem = g_locale_to_utf8(index->overview, -1, NULL, NULL, NULL);
					gtk_list_store_set(listStore, &iter, ERRORS_COLUMN_OVERVIEW, listitem, -1);
					g_free(listitem);
					listitem = NULL;
				}
				break;
			}
			default:
				break;
		}
	}
}

void SludgeProjectManager::readIniFile() {
	if (g_chdir(g_get_user_config_dir())) return;
#ifdef __WIN32
	_mkdir("sludge-devkit");
#else
	g_mkdir("sludge-devkit", 0000777);
#endif
	g_chdir("sludge-devkit");

	FILE * fp = fopen("SLUDGE.ini", "rb");
	
	programSettings.compilerKillImages = 0;
	programSettings.compilerWriteStrings = 0;
	programSettings.compilerVerbose = 1;
	programSettings.searchSensitive = 0;
#ifdef __WIN32
	sprintf(editor, "%s", "notepad.exe");
	sprintf(imageViewer, "%s", "rundll32.exe shimgvw.dll,ImageView_Fullscreen");
	sprintf(audioPlayer, "%s", "mplay32.exe");
	sprintf(modPlayer, "%s", "");
#else
	sprintf(editor, "%s", "gedit");
	sprintf(imageViewer, "%s", "eog");
	sprintf(audioPlayer, "%s", "totem");
	sprintf(modPlayer, "%s", "totem");
#endif
		
	if (fp) {
		char lineSoFar[257] = "";
		char secondSoFar[257] = "";
		unsigned char here = 0;
		char readChar = ' ';
		bool keepGoing = true;
		bool doingSecond = false;
		
		do {
			readChar = fgetc(fp);
			if (feof(fp)) {
				readChar = '\x0D';
				keepGoing = false;
			}
			switch (readChar) {
				case '\x0D':
				case '\x0A':

					if (!keepGoing) {
						fprintf(fp, "KillImages=%d\x0D\x0A", programSettings.compilerKillImages);
						fprintf(fp, "WriteStrings=%d\x0D\x0A", programSettings.compilerWriteStrings);
						fprintf(fp, "Verbose=%d\x0D\x0A", programSettings.compilerVerbose);
						fprintf(fp, "SearchSensitive=%d\x0D\x0A", programSettings.searchSensitive);
						fprintf(fp, "Editor=%s\x0D\x0A", editor);
						fprintf(fp, "ImageViewer=%s\x0D\x0A", imageViewer);
						fprintf(fp, "AudioPlayer=%s\x0D\x0A", audioPlayer);
						fprintf(fp, "ModPlayer=%s\x0D\x0A", modPlayer);
					}
					
					if (doingSecond) {
						if (strcmp(lineSoFar, "KillImages") == 0)
						{
							programSettings.compilerKillImages = atoi(secondSoFar);
						}
						else if (strcmp(lineSoFar, "WriteStrings") == 0)
						{
							programSettings.compilerWriteStrings = atoi(secondSoFar);
						}
						else if (strcmp(lineSoFar, "Verbose") == 0)
						{
							programSettings.compilerVerbose = atoi(secondSoFar);
						}
						else if (strcmp(lineSoFar, "SearchSensitive") == 0)
						{
							programSettings.searchSensitive = atoi(secondSoFar);
						}
						else if (strcmp(lineSoFar, "Editor") == 0)
						{
							sprintf(editor, "%s", secondSoFar);
						}
						else if (strcmp(lineSoFar, "ImageViewer") == 0)
						{
							sprintf(imageViewer, "%s", secondSoFar);
						}
						else if (strcmp(lineSoFar, "AudioPlayer") == 0)
						{
							sprintf(audioPlayer, "%s", secondSoFar);
						}
						else if (strcmp(lineSoFar, "ModPlayer") == 0)
						{
							sprintf(modPlayer, "%s", secondSoFar);
						}
					}
					here = 0;
					doingSecond = false;
					lineSoFar[0] = 0;
					secondSoFar[0] = 0;
					break;
					
				case '=':
					doingSecond = true;
					here = 0;
					break;
					
				default:
					if (doingSecond) {
						secondSoFar[here ++] = readChar;
						secondSoFar[here] = 0;
					} else {
						lineSoFar[here ++] = readChar;
						lineSoFar[here] = 0;
					}
					break;
			}
		} while (keepGoing);
		
		fclose(fp);
	}
	g_chdir(workingDir);
}

void SludgeProjectManager::saveIniFile() {
	if (g_chdir(g_get_user_config_dir())) return;
#ifdef __WIN32
	_mkdir("sludge-devkit");
#else
	g_mkdir("sludge-devkit", 0000777);
#endif
	g_chdir("sludge-devkit");
	
	FILE * fp = fopen("SLUDGE.ini", "wb");
	
	fprintf(fp, "KillImages=%d\x0D\x0A", programSettings.compilerKillImages);
	fprintf(fp, "WriteStrings=%d\x0D\x0A", programSettings.compilerWriteStrings);
	fprintf(fp, "Verbose=%d\x0D\x0A", programSettings.compilerVerbose);
	fprintf(fp, "SearchSensitive=%d\x0D\x0A", programSettings.searchSensitive);
	fprintf(fp, "Editor=%s\x0D\x0A", editor);
	fprintf(fp, "ImageViewer=%s\x0D\x0A", imageViewer);
	fprintf(fp, "AudioPlayer=%s\x0D\x0A", audioPlayer);
	fprintf(fp, "ModPlayer=%s\x0D\x0A", modPlayer);
	fclose(fp);
	g_chdir(workingDir);
}

void SludgeProjectManager::compile()
{
	compileEverything(currentFilename, fileList, &fileListNum, &receiveCompilerInfo);
}

void SludgeProjectManager::update_compile_window()
{
	char intbuf[100];

	compilerInfo *info = (compilerInfo *) g_async_queue_pop(compilerInfoQueue);

	if (info->progress1 > -1.)
		gtk_progress_bar_set_fraction(compProgress1, info->progress1);
	if (info->progress2 > -1.)
		gtk_progress_bar_set_fraction(compProgress2, info->progress2);
	if (info->task[0] != 0)
		gtk_label_set_text(compTask, info->task);
	if (info->file[0] != 0)
		gtk_label_set_text(compFile, info->file);
	if (info->item[0] != 0)
		gtk_label_set_text(compItem, info->item);
	if (info->funcs > -1) {
		sprintf(intbuf, "%i", info->funcs);
		gtk_label_set_text(compFuncs, intbuf);
	}
	if (info->objs > -1) {
		sprintf(intbuf, "%i", info->objs);
		gtk_label_set_text(compObjs, intbuf);
	}
	if (info->globs > -1) {
		sprintf(intbuf, "%i", info->globs);
		gtk_label_set_text(compGlobs, intbuf);
	}
	if (info->strings > -1) {
		sprintf(intbuf, "%i", info->strings);
		gtk_label_set_text(compStrings, intbuf);
	}
	if (info->resources > -1) {
		sprintf(intbuf, "%i", info->resources);
		gtk_label_set_text(compResources, intbuf);
	}
	if (info->newComments) {
		listChanged(ERROR_TREEVIEW);
	}
	if (info->finished) {
		gtk_label_set_text(compFile, "");
		gtk_label_set_text(compItem, "");
		gtk_widget_set_sensitive(closeCompilerButton, TRUE);
		if (info->success) {
			gtk_widget_set_sensitive(runGameButton, TRUE);
			gtk_widget_set_sensitive(projectRunGameItem, TRUE);
		} else {
			gtk_widget_set_sensitive(projectRunGameItem, FALSE);
		}
	}
	delete info;
}

// Callbacks:
void SludgeProjectManager::on_treeview_realize(GtkTreeView *theTreeView, whichTreeview whichOne)
{
	GtkTreeModel *sortModel;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	char caption[100];

	switch (whichOne) {
		case FILE_TREEVIEW:
			filesListStore = gtk_list_store_new(1, G_TYPE_STRING);
			sortModel = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(filesListStore));
			sprintf(caption, "Files in project");

			filesSelection = gtk_tree_view_get_selection(theTreeView);
			gtk_tree_selection_set_mode(filesSelection, GTK_SELECTION_MULTIPLE);
			g_signal_connect(G_OBJECT (filesSelection), "changed",
						      G_CALLBACK (on_files_tree_selection_changed_cb),
						      NULL);
			break;
		case RESOURCE_TREEVIEW:
			resourcesListStore = gtk_list_store_new(1, G_TYPE_STRING);
			sortModel = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(resourcesListStore));
			sprintf(caption, "Resources used by selected scripts");
			break;
		case ERROR_TREEVIEW:
			errorsListStore = gtk_list_store_new(ERRORS_N_COLUMNS, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);
			sortModel = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(errorsListStore));
			sprintf(caption, "Compiler errors");
			break;
		default:
			break;
	}

	gtk_tree_view_set_model(theTreeView, sortModel);

//  Sorting lists alphabetically is commented out,
//	because the order is important in some cases:
//	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE (sortModel),
//		                                    0, GTK_SORT_ASCENDING);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(caption,
		                                               renderer,
		                                               "text", 0,
		                                               NULL);
	gtk_tree_view_append_column(theTreeView, column);
}

void SludgeProjectManager::on_files_tree_selection_changed(GtkTreeSelection *theSelection)
{
	GList * selectedRows;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *tx, *tx1;

	clearFileList(resourceList, &numResources);
	if (currentFilename[0] == 0)
		return;
	getSourceDirFromName(currentFilename);

	selectedRows = gtk_tree_selection_get_selected_rows(theSelection, &model);

	for (int j = 0; j < g_list_length(selectedRows); j++)
	{
		gtk_tree_model_get_iter(model, &iter, (GtkTreePath *)g_list_nth(selectedRows, j)->data);
		gtk_tree_model_get(model, &iter, 0, &tx1, -1);
		tx = g_locale_from_utf8(tx1, -1, NULL, NULL, NULL);
		g_free(tx1);
		populateResourceList(tx, resourceList, &numResources);
		g_free(tx);

		gtk_widget_set_sensitive(removeFileButton, TRUE);
	}
	if (g_list_length(selectedRows) == 0) {
		gtk_widget_set_sensitive(removeFileButton, FALSE);
	}
	g_list_foreach(selectedRows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(selectedRows);

	listChanged(RESOURCE_TREEVIEW);
}

void SludgeProjectManager::on_treeview_row_activated(GtkTreeView *theTreeView, GtkTreePath *thePath, GtkTreeViewColumn *theColumn, whichTreeview whichOne)
{
	GtkTreeModel *treeModel;
	const char *cmd;	
	char *tx, *tx1;
	if (currentFilename[0] == 0)
		return;
	getSourceDirFromName(currentFilename);
	gotoSourceDirectory();

	treeModel = gtk_tree_view_get_model(theTreeView);

	int filenameColumn = 0;
	switch (whichOne) {
		case ERROR_TREEVIEW:
			filenameColumn = ERRORS_COLUMN_FILENAME;
		case FILE_TREEVIEW:
			cmd = editor;
			break;
		default:
			break;
	}

	GtkTreeIter iter;
	gtk_tree_model_get_iter(treeModel, &iter, thePath);

	if (whichOne == ERROR_TREEVIEW) {
		gboolean hasFilename;
		gtk_tree_model_get(treeModel, &iter, ERRORS_COLUMN_HAS_FILENAME, &hasFilename, -1);
		if (!hasFilename) {
			gtk_tree_model_get(treeModel, &iter, ERRORS_COLUMN_OVERVIEW, &tx, -1);
			errorBox("Error overview", tx);
  			g_free(tx);
			return;
		}
	}

	gtk_tree_model_get(treeModel, &iter, filenameColumn, &tx1, -1);
	tx = g_filename_from_utf8(tx1, -1, NULL, NULL, NULL);
    g_free(tx1);

	char * extension;
	gint *indices;
	struct errorLinkToFile * index;
	int i, row;
	switch (whichOne) {
		case FILE_TREEVIEW:
			extension = tx + strlen(tx) - 4;
			if ((strlen(tx) > 4) && (!strcmp(extension, ".tra") || !strcmp(extension, ".TRA"))) {
#ifdef __WIN32
				cmd = "sludge-translationeditor.exe";	
#else
				cmd = "sludge-translationeditor";	
#endif
			}
			break;
		case RESOURCE_TREEVIEW:
			extension = tx + strlen(tx) - 4;	
			if (strlen(tx) > 4) {
				if        (!strcmp(extension, ".flo") || !strcmp(extension, ".FLO")) {
#ifdef __WIN32
					cmd = "sludge-floormaker.exe";
#else
					cmd = "sludge-floormaker";
#endif
				} else if (!strcmp(extension, ".duc") || !strcmp(extension, ".DUC")) {
#ifdef __WIN32
					cmd = "sludge-spritebankeditor.exe";	
#else
					cmd = "sludge-spritebankeditor";	
#endif
				} else if (!strcmp(extension, ".zbu") || !strcmp(extension, ".ZBU")) {
#ifdef __WIN32
					cmd = "sludge-zbuffermaker.exe";	
#else
					cmd = "sludge-zbuffermaker";	
#endif
				} else if (!strcmp(extension, ".png") || !strcmp(extension, ".PNG")
						|| !strcmp(extension, ".tga") || !strcmp(extension, ".TGA")) {
					cmd = imageViewer;
				} else if (!strcmp(extension, ".wav") || !strcmp(extension, ".WAV")
						|| !strcmp(extension, ".ogg") || !strcmp(extension, ".OGG")
						|| !strcmp(extension, "flac") || !strcmp(extension, "FLAC")) {
					cmd = audioPlayer;
				} else if (!strcmp(extension, ".mod") || !strcmp(extension, ".MOD")
						|| !strcmp(extension, ".s3m") || !strcmp(extension, ".S3M")) {
					cmd = modPlayer;
				} else {
					extension = tx + strlen(tx) - 3;
					if (strlen(tx) > 3 &&
						  (!strcmp(extension, ".it") || !strcmp(extension, ".IT")
						|| !strcmp(extension, ".xm") || !strcmp(extension, ".XM"))) {
						cmd = modPlayer;
					}
				}
			}
			break;
		default:
			break;
	}

	if (cmd[0] != 0 && tx) {
		sh_cmd(workingDir, cmd, getFullPath(tx));
	}
/*
	else {
		char buf[1000];
		GError *error = NULL;
		sprintf(buf, "file://%s", getFullPath(tx));
		gtk_show_uri (NULL, buf, GDK_CURRENT_TIME, &error);
		if (error != NULL) {
			errorBox("Opening file failed!", error->message);
			g_error_free(error);
		}
	}
*/

	g_free(tx);
}

void SludgeProjectManager::on_add_file_clicked()
{
	GtkWidget *dialog;
	GtkFileFilter *filter, *slufilter, *sldfilter, *trafilter;

	dialog = gtk_file_chooser_dialog_new("Add file to SLUDGE project",
				      NULL,
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "SLU/SLD/TRA files");
	gtk_file_filter_add_pattern(filter, "*.[sS][lL][uU]");
	gtk_file_filter_add_pattern(filter, "*.[sS][lL][dD]");
	gtk_file_filter_add_pattern(filter, "*.[tT][rR][aA]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), filter);

	slufilter = gtk_file_filter_new();
	gtk_file_filter_set_name(slufilter, "SLUDGE scripts (*.slu)");
	gtk_file_filter_add_pattern(slufilter, "*.[sS][lL][uU]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), slufilter);

	sldfilter = gtk_file_filter_new();
	gtk_file_filter_set_name(sldfilter, "SLUDGE constant definition files (*.sld)");
	gtk_file_filter_add_pattern(sldfilter, "*.[sS][lL][dD]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), sldfilter);

	trafilter = gtk_file_filter_new();
	gtk_file_filter_set_name(trafilter, "SLUDGE translation files (*.tra)");
	gtk_file_filter_add_pattern(trafilter, "*.[tT][rR][aA]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER (dialog), trafilter);

	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER (dialog), filter);

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
			filename = (char *) g_slist_nth(filenames, j)->data;
			flipBackslashes(&filename);

			getSourceDirFromName(currentFilename);
			addFileToProject(filename, sourceDirectory, fileList, &fileListNum);
			setFileChanged();
		
			g_free(filename);
		}
		g_slist_free(filenames);
	}
	gtk_widget_destroy(dialog);

	listChanged(FILE_TREEVIEW);
}

void SludgeProjectManager::on_remove_file_clicked()
{
	GList * selectedRows;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	gchar *indexStr;
	int index;

	if (! askAQuestion("Remove files?", "Do you want to remove the selected files from the project? (They will not be deleted from the disk.)")) {
		return;
	}

	selectedRows = gtk_tree_selection_get_selected_rows(filesSelection, &model);

	for (int j = g_list_length(selectedRows) - 1; j >= 0; j--)
	{
		path = gtk_tree_model_sort_convert_path_to_child_path(
					GTK_TREE_MODEL_SORT(model),
					(GtkTreePath *)g_list_nth(selectedRows, j)->data);

		indexStr = gtk_tree_path_to_string(path);
		index = atoi(indexStr);
        g_free(indexStr);

		removeFileFromList(index, fileList, &fileListNum);
		setFileChanged();

		gtk_tree_model_get_iter(GTK_TREE_MODEL(filesListStore), &iter, path);
		gtk_list_store_remove(filesListStore, &iter);
    }
	g_list_foreach(selectedRows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(selectedRows);
}

void SludgeProjectManager::on_compile()
{
	//Switch to the "Compiler Errors" notebook to make sure the TableView is realized:
	gtk_notebook_set_current_page(notebook, 1); 

	gtk_widget_set_sensitive(closeCompilerButton, FALSE);
	gtk_widget_set_sensitive(runGameButton, FALSE);
	gtk_widget_show(GTK_WIDGET(compilerDialog));

	compilerInfoQueue = g_async_queue_new();

	GError *error = NULL;

	gdk_threads_leave ();
	g_thread_create(compile_hook, NULL, FALSE, &error);
	gdk_threads_enter ();

	if (error != NULL) {
		errorBox("Could not create a new thread for compiling!", error->message);
		g_error_free(error);
		return;
	}
}

void SludgeProjectManager::on_comp_okbutton_clicked(GtkButton *theButton)
{
	GtkWidget *theDialog;
	theDialog = gtk_widget_get_toplevel(GTK_WIDGET(theButton));

	gtk_widget_hide(theDialog);
}

void SludgeProjectManager::on_comp_gamebutton_clicked()
{
	char cmd[1000];
	char *file = getFullPath(gameFile);

#ifdef __WIN32
	int cmdSuccess;
	int lastSlash = 0;
	char engineDir[1000];
	sprintf(engineDir, "%s", workingDir);

	for (int k = 0; engineDir[k] != 0; k++) {
		if (engineDir[k] == '\\') {
			lastSlash = k;
		}
	}
	sprintf(engineDir + lastSlash + 1, "Engine");
	sprintf(cmd, "\"%s\\SLUDGE Engine.exe\"", engineDir);
	cmdSuccess = sh_cmd(engineDir, cmd, file);

	if (!cmdSuccess) {
		errorBox("Couldn't find SLUDGE Engine", "The SLUDGE Engine was expected at ..\\Engine\\SLUDGE Engine.exe relative to the Project Manager executable. Make sure it's there to make the Run Game button work.");
	}
#else
	sprintf(cmd, "%s", "sludge-engine");
	sh_cmd(workingDir, cmd, file);
#endif

	deleteString(file);
}

void SludgeProjectManager::on_project_settings()
{
	gtk_entry_set_text(prefName, settings.windowName ? settings.windowName : "");
	gtk_entry_set_text(prefQuit, settings.quitMessage ? settings.quitMessage : "");
	gtk_entry_set_text(prefSave, settings.runtimeDataFolder ? settings.runtimeDataFolder : "");
	gtk_entry_set_text(prefLanguage, settings.originalLanguage ? settings.originalLanguage : "");
	gtk_entry_set_text(prefFilename, settings.finalFile ? settings.finalFile : "");
	gtk_entry_set_text(prefIcon, settings.customIcon ? settings.customIcon : "");
	gtk_entry_set_text(prefLogo, settings.customLogo ? settings.customLogo : "");
	gtk_spin_button_set_value(prefWidth, (double)settings.screenWidth);
	gtk_spin_button_set_value(prefHeight, (double)settings.screenHeight);
	gtk_spin_button_set_value(prefSpeed, (double)settings.frameSpeed);
	gtk_toggle_button_set_active(prefSilent, settings.forceSilent);

	if (gtk_dialog_run(projectSettingsDialog) == GTK_RESPONSE_OK)
	{
		killSettingsStrings();
		settings.quitMessage = copyString(gtk_entry_get_text(prefQuit));
		settings.customIcon = copyString(gtk_entry_get_text(prefIcon));
		settings.customLogo = copyString(gtk_entry_get_text(prefLogo));
		settings.runtimeDataFolder = copyString(gtk_entry_get_text(prefSave));
		settings.finalFile = copyString(gtk_entry_get_text(prefFilename));
		settings.windowName = copyString(gtk_entry_get_text(prefName));
		settings.originalLanguage = copyString(gtk_entry_get_text(prefLanguage));
		settings.screenHeight = gtk_spin_button_get_value_as_int(prefHeight);
		settings.screenWidth = gtk_spin_button_get_value_as_int(prefWidth);
		settings.frameSpeed = gtk_spin_button_get_value_as_int(prefSpeed);
		settings.forceSilent = gtk_toggle_button_get_active(prefSilent);
		setFileChanged();
	}
	gtk_widget_hide(GTK_WIDGET(projectSettingsDialog));
}

void SludgeProjectManager::on_preferences()
{
	gtk_toggle_button_set_active(prefKeepImages, !programSettings.compilerKillImages);
	gtk_toggle_button_set_active(prefWriteStrings, programSettings.compilerWriteStrings);
	gtk_toggle_button_set_active(prefVerbose, programSettings.compilerVerbose);
	gtk_entry_set_text(prefEditor, editor ? editor : "");
	gtk_entry_set_text(prefImageViewer, imageViewer ? imageViewer : "");
	gtk_entry_set_text(prefAudioPlayer, audioPlayer ? audioPlayer : "");
	gtk_entry_set_text(prefModPlayer, modPlayer ? modPlayer : "");

	if (gtk_dialog_run(preferenceDialog) == GTK_RESPONSE_OK)
	{
		programSettings.compilerKillImages = !gtk_toggle_button_get_active(prefKeepImages);
		programSettings.compilerWriteStrings = gtk_toggle_button_get_active(prefWriteStrings);
		programSettings.compilerVerbose = gtk_toggle_button_get_active(prefVerbose);
		sprintf(editor, "%s", gtk_entry_get_text(prefEditor));
		sprintf(imageViewer, "%s", gtk_entry_get_text(prefImageViewer));
		sprintf(audioPlayer, "%s", gtk_entry_get_text(prefAudioPlayer));
		sprintf(modPlayer, "%s", gtk_entry_get_text(prefModPlayer));

		saveIniFile();
	}
	gtk_widget_hide(GTK_WIDGET(preferenceDialog));
}

void SludgeProjectManager::on_program_activate(whichProgram program)
{
	const char *exe;
	char executable[200];
#ifdef __WIN32
	exe = ".exe";
#else
	exe = "";
#endif

	switch (program) {
		case FLOORMAKER:
			sprintf(executable, "sludge-floormaker%s", exe);
			break;
		case SPRITEBANKEDITOR:
			sprintf(executable, "sludge-spritebankeditor%s", exe);
			break;
		case ZBUFFERMAKER:
			sprintf(executable, "sludge-zbuffermaker%s", exe);
			break;
		case TRANSLATIONEDITOR:
			sprintf(executable, "sludge-translationeditor%s", exe);
			break;
		default:
			break;
	}

	sh_cmd(workingDir, executable, NULL);
}
