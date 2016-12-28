/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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

#ifndef __HB_CATEGORY_GTK_H__
#define __HB_CATEGORY_GTK_H__

enum
{
	LST_DEFCAT_TOGGLE,
	LST_DEFCAT_DATAS,
	LST_DEFCAT_NAME,
	NUM_LST_DEFCAT
};

#define LST_DEFCAT_SORT_NAME 1
#define LST_DEFCAT_SORT_USED 2


enum
{
	CAT_TYPE_ALL,
	CAT_TYPE_EXPENSE,
	CAT_TYPE_INCOME
};


enum
{
	LST_CMBCAT_DATAS,
	LST_CMBCAT_FULLNAME,
	LST_CMBCAT_SORTNAME,
	LST_CMBCAT_NAME,
	LST_CMBCAT_SUBCAT,
	NUM_LST_CMBCAT
};


gchar *ui_cat_comboboxentry_get_name(GtkComboBox *entry_box);
guint32 ui_cat_comboboxentry_get_key(GtkComboBox *entry_box);
guint32 ui_cat_comboboxentry_get_key_add_new(GtkComboBox *entry_box);
Category *ui_cat_comboboxentry_get(GtkComboBox *entry_box);

gboolean ui_cat_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key);
void ui_cat_comboboxentry_add(GtkComboBox *entry_box, Category *pay);
void ui_cat_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash);
void ui_cat_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key);
GtkWidget *ui_cat_comboboxentry_new(GtkWidget *label);

/* = = = = = = = = = = */

void ui_cat_listview_add(GtkTreeView *treeview, Category *item, GtkTreeIter	*parent);
Category *ui_cat_listview_get_selected(GtkTreeView *treeview);
Category *ui_cat_listview_get_selected_parent(GtkTreeView *treeview, GtkTreeIter *parent);
gboolean ui_cat_listview_remove (GtkTreeModel *liststore, guint32 key);
void ui_cat_listview_remove_selected(GtkTreeView *treeview);
void ui_cat_listview_populate(GtkWidget *view, gint type);
GtkWidget *ui_cat_listview_new(gboolean withtoggle, gboolean withcount);

/* = = = = = = = = = = */

struct ui_cat_manage_dialog_data
{
	GList	*tmp_list;
	gint	change;

	GtkWidget	*window;

	GtkWidget	*LV_cat;
	GtkWidget	*ST_name1, *ST_name2;

	//GtkWidget	*BT_add1, *BT_add2;

	//GtkWidget	*CM_type;
	GtkWidget	*RA_type;

	GtkWidget	*BT_edit;
	GtkWidget	*BT_merge;
	GtkWidget	*BT_delete;
	
	GtkWidget	*BT_expand;
	GtkWidget	*BT_collapse;

	GtkWidget	*LA_category;

};

struct catPopContext
{
	GtkTreeModel *model;
	guint	except_key;
	gint	type;
};

GtkWidget *ui_cat_manage_dialog (void);

#endif

