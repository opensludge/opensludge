/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * TranslationEditorMain.h - Part of the SLUDGE Translation Editor (GTK+ version)
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

#ifdef __cplusplus
extern "C" {
#endif
G_MODULE_EXPORT void
on_tree_selection_changed_cb(GtkTreeSelection *theSelection, gpointer theUser_data);
G_MODULE_EXPORT void
on_translate_toggled_cb(GtkCellRendererToggle *theCell_renderer, gchar *thePath, gpointer theUser_data);
G_MODULE_EXPORT void
on_translation_edited_cb(GtkCellRendererText *theCell_renderer, gchar *thePath, gchar *theNewText, gpointer theUser_data);
G_MODULE_EXPORT void
on_sort_original_clicked_cb(GtkTreeViewColumn *theTreeViewColumn, gpointer theUser_data);
G_MODULE_EXPORT void
on_sort_translation_clicked_cb(GtkTreeViewColumn *theTreeViewColumn, gpointer theUser_data);
#ifdef __cplusplus
}
#endif
