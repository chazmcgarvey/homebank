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

#ifndef __HB_ACCOUNT_GTK_H__
#define __HB_ACCOUNT_GTK_H__

enum
{
	LST_DEFACC_TOGGLE,
	LST_DEFACC_DATAS,
	NUM_LST_DEFACC
};

enum
{
	ACC_LST_INSERT_NORMAL,
	ACC_LST_INSERT_REPORT
};


gchar *ui_acc_comboboxentry_get_name(GtkComboBox *entry_box);
guint32 ui_acc_comboboxentry_get_key(GtkComboBox *entry_box);
gboolean ui_acc_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key);
void ui_acc_comboboxentry_add(GtkComboBox *entry_box, Account *acc);
void ui_acc_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash, gint insert_type);
void ui_acc_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key, gint insert_type);
GtkWidget *ui_acc_comboboxentry_new(GtkWidget *label);

/* = = = = = = = = = = */

void ui_acc_listview_add(GtkTreeView *treeview, Account *item);
guint32 ui_acc_listview_get_selected_key(GtkTreeView *treeview);
void ui_acc_listview_remove_selected(GtkTreeView *treeview);
void ui_acc_listview_populate(GtkWidget *view, gint insert_type);
GtkWidget *ui_acc_listview_new(gboolean withtoggle);

/* = = = = = = = = = = */

enum
{
	ACTION_NEW,
	ACTION_MODIFY,
	ACTION_REMOVE,
};

enum
{
	FIELD_NAME,
	//todo: for stock account	
	//FIELD_TYPE,
	FIELD_BANK,
	FIELD_NUMBER,
	FIELD_BUDGET,
	FIELD_CLOSED,
	FIELD_INITIAL,
	FIELD_MINIMUM,
	FIELD_CHEQUE1,
	FIELD_CHEQUE2,
	MAX_ACC_FIELD
};


struct ui_acc_manage_data
{
	GList	*tmp_list;
	gint	change;
	gint	action;
	guint32	lastkey;

	GtkWidget	*window;

	GtkWidget	*LV_acc;
	GtkWidget	*BT_add, *BT_rem;

	GtkWidget	*CY_curr;
	GtkWidget	*CY_type;

	GtkWidget   *notebook;
	
	GtkWidget	*ST_bank;
	GtkWidget	*ST_number;
	GtkWidget	*CM_nosummary;
	GtkWidget	*CM_nobudget;
	GtkWidget	*CM_noreport;
	GtkWidget	*CM_closed;
	GtkWidget	*ST_initial;
	GtkWidget	*ST_warning;
	GtkWidget	*ST_overdraft;
	GtkWidget	*ST_cheque1;
	GtkWidget	*ST_cheque2;
	GtkWidget	*TB_notes;
	
};


struct accPopContext
{
	GtkTreeModel *model;
	guint32	except_key;
	guint32 kcur;
	gint	insert_type;
};


GtkWidget *ui_acc_manage_dialog (void);


#endif

