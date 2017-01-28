/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2017 Maxime DOYEN
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
	LIST_TXN_TYPE_BOOK,
	LIST_TXN_TYPE_DETAIL,
	LIST_TXN_TYPE_IMPORT,
};

struct list_txn_data
{
	GtkWidget			*treeview;
	GtkTreeViewColumn   *tvc_balance;
	
	gint				list_type;
	gboolean			tvc_is_visible;
	gboolean			save_column_width;
};



GtkWidget *create_list_transaction(gint type, gboolean *pref_columns);
GtkWidget *create_list_import_transaction(gboolean enable_choose);

gboolean list_txn_column_id_isvisible(GtkTreeView *treeview, gint sort_id);

Transaction *list_txn_get_active_transaction(GtkTreeView *treeview);

void list_txn_set_save_column_width(GtkTreeView *treeview, gboolean save_column_width);
void list_txn_sort_force(GtkTreeSortable *sortable, gpointer user_data);
guint list_txn_get_quicksearch_column_mask(GtkTreeView *treeview);

#endif
