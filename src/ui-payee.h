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

#ifndef __HB_PAYEE_GTK_H__
#define __HB_PAYEE_GTK_H__

enum
{
	LST_DEFPAY_TOGGLE,
	LST_DEFPAY_DATAS,
	NUM_LST_DEFPAY
};

struct ui_pay_manage_dialog_data
{
	GtkWidget	*window;

	GtkWidget	*ST_name;
	GtkWidget	*LV_pay;

	GtkWidget	*BT_add;
	GtkWidget	*BT_mov;
	GtkWidget	*BT_mod;
	GtkWidget	*BT_rem;
	GtkWidget	*BT_import;
	GtkWidget	*BT_export;

	gint	change;
};

struct payPopContext
{
	GtkTreeModel *model;
	guint	except_key;
};


gchar *ui_pay_comboboxentry_get_name(GtkComboBox *entry_box);
guint32 ui_pay_comboboxentry_get_key(GtkComboBox *entry_box);
guint32 ui_pay_comboboxentry_get_key_add_new(GtkComboBox *entry_box);
gboolean ui_pay_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key);
void ui_pay_comboboxentry_add(GtkComboBox *entry_box, Payee *pay);
void ui_pay_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash);
void ui_pay_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key);
GtkWidget *ui_pay_comboboxentry_new(GtkWidget *label);

/* = = = = = = = = = = */

void ui_pay_listview_add(GtkTreeView *treeview, Payee *item);
guint32 ui_pay_listview_get_selected_key(GtkTreeView *treeview);
void ui_pay_listview_remove_selected(GtkTreeView *treeview);
void ui_pay_listview_populate(GtkWidget *view);
GtkWidget *ui_pay_listview_new(gboolean withtoggle);
GtkWidget *ui_pay_manage_dialog (void);

#endif

