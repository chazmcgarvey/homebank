/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2019 Maxime DOYEN
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

#ifndef __HB_PAYEE_GTK_H__
#define __HB_PAYEE_GTK_H__

enum
{
	LST_DEFPAY_TOGGLE,
	LST_DEFPAY_DATAS,
	NUM_LST_DEFPAY
};

#define LST_DEFPAY_SORT_NAME	 1
#define LST_DEFPAY_SORT_USED	 2
#define LST_DEFPAY_SORT_DEFCAT   3

struct ui_pay_manage_dialog_data
{
	GtkWidget	*window;
	
	GtkWidget	*ST_search;
	GtkWidget	*BT_search;
	GtkWidget	*ST_name;
	GtkWidget	*LV_pay;

	GtkWidget	*BT_add;
	GtkWidget	*BT_edit;
	GtkWidget	*BT_merge;
	GtkWidget	*BT_delete;

	gint		change;
};


struct payPopContext
{
	GtkTreeModel *model;
	guint	except_key;
};



/* = = = = = = = = = = */

GtkWidget *ui_pay_entry_popover_get_entry(GtkBox *box);
Payee *ui_pay_entry_popover_get(GtkBox *box);
guint32 ui_pay_entry_popover_get_key_add_new(GtkBox *box);
guint32 ui_pay_entry_popover_get_key(GtkBox *box);
void ui_pay_entry_popover_set_active(GtkBox *box, guint32 key);
GtkWidget *ui_pay_entry_popover_new(GtkWidget *label);


/* = = = = = = = = = = */

guint32 ui_pay_comboboxentry_get_key(GtkComboBox *entry_box);
guint32 ui_pay_comboboxentry_get_key_add_new(GtkComboBox *entry_box);
Payee *ui_pay_comboboxentry_get(GtkComboBox *entry_box);
gboolean ui_pay_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key);
void ui_pay_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash);
GtkWidget *ui_pay_comboboxentry_new(GtkWidget *label);

/* = = = = = = = = = = */

void ui_pay_listview_add(GtkTreeView *treeview, Payee *item);
guint32 ui_pay_listview_get_selected_key(GtkTreeView *treeview);
void ui_pay_listview_remove_selected(GtkTreeView *treeview);
void ui_pay_listview_populate(GtkWidget *treeview, gchar *needle);
GtkWidget *ui_pay_listview_new(gboolean withtoggle, gboolean withcount);

GtkWidget *ui_pay_manage_dialog (void);

#endif

