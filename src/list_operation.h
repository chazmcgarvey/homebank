/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2014 Maxime DOYEN
 *
 *  This file is part of HomeBank.
 *
 *  HomeBank is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  HomeBank is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIST_OPERATION__H__
#define __LIST_OPERATION__H__

enum {
	TRN_LIST_TYPE_BOOK,
	TRN_LIST_TYPE_DETAIL,
	TRN_LIST_TYPE_IMPORT,
};

struct list_transaction_data
{
	GtkTreeViewColumn   *tvc_balance;
	gboolean			tvc_is_visible;
};


GtkWidget *create_list_transaction(gint type, gboolean *pref_columns);
void list_transaction_sort_force(GtkTreeSortable *sortable, gpointer user_data);
guint list_transaction_get_quicksearch_column_mask(GtkTreeView *treeview);
GtkWidget *create_list_import_transaction(void);

#endif
