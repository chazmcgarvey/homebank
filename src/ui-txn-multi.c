/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2018 Maxime DOYEN
 *
 *	This file is part of HomeBank.
 *
 *	HomeBank is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	HomeBank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-txn-multi.h"

#include "ui-account.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "gtk-dateentry.h"
#include "list_operation.h"


/****************************************************************************/
/* Debug macros											                    */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


void ui_multipleedit_dialog_prefill( GtkWidget *widget, Transaction *ope, gint column_id )
{
struct ui_multipleedit_dialog_data *data;
gchar *tagstr;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[ui-multipleedit] prefill\n") );

	if(ope != NULL)
	//if(col_id >= LST_DSPOPE_DATE && col_id != LST_DSPOPE_BALANCE)
	{
		switch( column_id )
		{
			case LST_DSPOPE_DATE:
				gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_date), (guint)ope->date);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_date), TRUE);
				break;
			case LST_DSPOPE_INFO:
				gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), ope->paymode);
				gtk_entry_set_text(GTK_ENTRY(data->ST_info), (ope->info != NULL) ? ope->info : "");
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_mode), TRUE);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_info), TRUE);
				break;
			case LST_DSPOPE_PAYEE:
				ui_pay_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_pay), ope->kpay);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_pay), TRUE);
				break;
			case LST_DSPOPE_MEMO:
				gtk_entry_set_text(GTK_ENTRY(data->ST_memo), (ope->memo != NULL) ? ope->memo : "");
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_memo), TRUE);
				break;
			case LST_DSPOPE_CATEGORY:
				ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat), ope->kcat);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_cat), TRUE);
				break;
			case LST_DSPOPE_TAGS:
				tagstr = transaction_tags_tostring(ope);
				gtk_entry_set_text(GTK_ENTRY(data->ST_tags), (tagstr != NULL) ? tagstr : "");
				g_free(tagstr);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(data->CM_tags), TRUE);
				break;
		}
	}
}


static void ui_multipleedit_dialog_update( GtkWidget *widget, gpointer user_data )
{
struct ui_multipleedit_dialog_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[ui-multipleedit] update\n") );

	if(data->PO_date)
		gtk_widget_set_sensitive (data->PO_date, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_date)) );

	if(data->NU_mode && data->ST_info)
	{
		gtk_widget_set_sensitive (data->NU_mode, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_mode)) );
		gtk_widget_set_sensitive (data->ST_info, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_info)) );
	}

	if(data->PO_acc)
		gtk_widget_set_sensitive (data->PO_acc , gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_acc )) );

	if(data->PO_pay)
		gtk_widget_set_sensitive (data->PO_pay , gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_pay )) );

	if(data->PO_cat)
		gtk_widget_set_sensitive (data->PO_cat , gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_cat )) );

	if(data->ST_tags)
		gtk_widget_set_sensitive (data->ST_tags, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_tags)) );

	if(data->ST_memo)
		gtk_widget_set_sensitive (data->ST_memo, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_memo)) );
}


static void ui_multipleedit_dialog_init( GtkWidget *widget, gpointer user_data )
{
struct ui_multipleedit_dialog_data *data;
GtkTreeModel *model;
GList *selection, *list;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[ui-multipleedit] init\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->treeview));
	selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->treeview)), &model);

	data->has_xfer = FALSE;

	list = g_list_last(selection);
	while(list != NULL)
	{
	Transaction *entry;
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &entry, -1);

		if(entry->paymode == PAYMODE_INTXFER)
			data->has_xfer = TRUE;

		list = g_list_previous(list);
	}

	g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(selection);

}


gint ui_multipleedit_dialog_apply( GtkWidget *widget, gpointer user_data )
{
struct ui_multipleedit_dialog_data *data;
GtkTreeModel *model;
GList *selection, *list;
guint changes;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[ui-multipleedit] apply\n") );

	changes = GLOBALS->changes_count; 

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->treeview));
	selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->treeview)), &model);

	list = g_list_last(selection);
	while(list != NULL)
	{
	Transaction *txn;
	GtkTreeIter iter;
	const gchar *txt;
	gboolean change = FALSE;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &txn, -1);

		DB( g_print(" modifying %s %.2f\n", txn->memo, txn->amount) );

		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_DATE) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_date)) )
			{
				txn->date = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_date));
				DB( g_print(" -> date: '%d'\n", txn->date) );
				change = TRUE;
			}
		}

		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_INFO) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_mode)) )
			{
				txn->paymode = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
				change = TRUE;
			}
		
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_info)) )
			{
				if(txn->info)
				{
					g_free(txn->info);
					txn->info = NULL;
					change = TRUE;
				}

				txt = gtk_entry_get_text(GTK_ENTRY(data->ST_info));
				if (txt && *txt)
				{
					txn->info = g_strdup(txt);
					change = TRUE;
				}
			}
		}

		if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_acc)) )
		{
		guint32 nkacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			
			if( transaction_acc_move(txn, txn->kacc, nkacc) )
			{
			GtkTreeIter iter;

				DB( g_print(" -> acc: '%d'\n", nkacc) );	
				gtk_tree_model_get_iter(model, &iter, list->data);
				gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
				change = TRUE;
			}
		}

		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_PAYEE) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_pay)) )
			{
				txn->kpay = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_pay));
				DB( g_print(" -> payee: '%d'\n", txn->kpay) );
				change = TRUE;
			}
		}

		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_CATEGORY) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_cat)) )
			{
				if(!(txn->flags & OF_SPLIT))
				{
					txn->kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_cat));
					DB( g_print(" -> category: '%d'\n", txn->kcat) );
					change = TRUE;
				}
			}
		}
		
		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_TAGS) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_tags)) )
			{
				txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tags));
				if (txt && *txt)
				{
					transaction_tags_parse(txn, txt);
					DB( g_print(" -> tags: '%s'\n", txt) );
					change = TRUE;
				}
			}
		}

		if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_MEMO) == TRUE )
		{
			if( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_memo)) )
			{
				if(txn->memo)
				{
					g_free(txn->memo);
					txn->memo = NULL;	
					change = TRUE;
				}

				txt = gtk_entry_get_text(GTK_ENTRY(data->ST_memo));
				if (txt && *txt)
				{
					txn->memo = g_strdup(txt);
					change = TRUE;
				}
			}
		}

		/* since 5.1 date and amount are no more editable
			case LST_DSPOPE_DATE:
				txn->date = gtk_date_entry_get_date(GTK_DATE_ENTRY(widget1));
				data->do_sort = TRUE;
				refreshbalance = TRUE;
				break;
			case LST_DSPOPE_EXPENSE:
			case LST_DSPOPE_INCOME:
			case LST_DSPOPE_AMOUNT:
				txn->flags &= ~(OF_INCOME);	//delete flag
				txn->amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget1));
				if(txn->amount > 0) txn->flags |= OF_INCOME;
				refreshbalance = TRUE;
				break;
		*/

		if( change == TRUE )
		{
			txn->flags |= OF_CHANGED;
			GLOBALS->changes_count++;
		}

		if( data->has_xfer && txn->paymode == PAYMODE_INTXFER )
		{
		Transaction *child;
			child = transaction_xfer_child_strong_get(txn);
			transaction_xfer_child_sync(txn, child);
		}

		list = g_list_previous(list);
	}

	g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(selection);

	return GLOBALS->changes_count - changes;
}


static gboolean ui_multipleedit_dialog_destroy( GtkWidget *widget, gpointer user_data )
{
struct ui_multipleedit_dialog_data *data;

	data = g_object_get_data(G_OBJECT(widget), "inst_data");

	DB( g_print ("\n[ui-multipleedit] destroy event occurred\n") );

	g_free(data);
	return FALSE;
}


GtkWidget *ui_multipleedit_dialog_new(GtkWindow *parent, GtkTreeView *treeview)
{
struct ui_multipleedit_dialog_data *data;
GtkWidget *dialog, *content_area;
GtkWidget *group_grid, *label, *widget, *toggle;
gint row;

	DB( g_print ("\n[ui-multipleedit] new\n") );

	data = g_malloc0(sizeof(struct ui_multipleedit_dialog_data));

	dialog = gtk_dialog_new_with_buttons (NULL,
						GTK_WINDOW (parent),
						0,
						_("_Cancel"),
						GTK_RESPONSE_REJECT,
						_("_OK"),
						GTK_RESPONSE_ACCEPT,
						NULL);

	//g_signal_connect (dialog, "delete_event", G_CALLBACK (register_panel_dispose), (gpointer)data);
	g_signal_connect (dialog, "destroy", G_CALLBACK (ui_multipleedit_dialog_destroy), (gpointer)data);

	//store our window private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", dialog, data) );

	data->window = dialog;
	data->treeview = treeview;

	ui_multipleedit_dialog_init(dialog, NULL);


	gtk_window_set_title (GTK_WINDOW (data->window), _("Multiple edit transactions"));

	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_container_set_border_width (GTK_CONTAINER(group_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (content_area), group_grid);

	row = -1;

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_DATE) == TRUE )
	{
		row++;
		label = make_label_widget(_("_Date:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_date = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = gtk_date_entry_new();
		data->PO_date = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_date , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_INFO) == TRUE )
	{
		row++;
		label = make_label_widget(_("Pa_yment:"));
		data->LB_mode = label;
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		toggle = gtk_check_button_new();
		data->CM_mode = toggle;
		gtk_grid_attach (GTK_GRID (group_grid), toggle, 1, row, 1, 1);
		widget = make_paymode_nointxfer (label);
		data->NU_mode = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_mode , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);

		row++;
		label = make_label_widget(_("_Info:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_info = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = make_string(label);
		data->ST_info = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_info , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}

	row++;
	label = make_label_widget(_("A_ccount:"));
	data->LB_acc = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = gtk_check_button_new();
	data->CM_acc = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);
	
	g_signal_connect (data->CM_acc , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_PAYEE) == TRUE )
	{
		row++;
		label = make_label_widget(_("_Payee:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_pay = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = ui_pay_comboboxentry_new(label);
		data->PO_pay = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_pay  , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}
	
	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_CATEGORY) == TRUE )
	{
		row++;
		label = make_label_widget(_("_Category:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_cat = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = ui_cat_comboboxentry_new(label);
		data->PO_cat = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_cat  , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_TAGS) == TRUE )
	{
		row++;
		label = make_label_widget(_("Ta_gs:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_tags = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = make_string(label);
		data->ST_tags = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_tags , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_MEMO) == TRUE )
	{
		row++;
		label = make_label_widget(_("M_emo:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = gtk_check_button_new();
		data->CM_memo = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
		widget = make_memo_entry(label);
		data->ST_memo = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (data->CM_memo , "toggled", G_CALLBACK (ui_multipleedit_dialog_update), NULL);
	}


	ui_multipleedit_dialog_update(dialog, NULL);

	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);
	ui_pay_comboboxentry_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->h_cat);

	gtk_widget_show_all (dialog);

	if(data->has_xfer == TRUE)
	{
		hb_widget_visible (data->LB_acc, FALSE);
		hb_widget_visible (data->CM_acc, FALSE);
		hb_widget_visible (data->PO_acc, FALSE);
	}

	if( list_txn_column_id_isvisible(GTK_TREE_VIEW(data->treeview), LST_DSPOPE_INFO) == TRUE )
	{
		if(data->has_xfer == TRUE)
		{
			hb_widget_visible (data->LB_mode, FALSE);
			hb_widget_visible (data->CM_mode, FALSE);
			hb_widget_visible (data->NU_mode, FALSE);
		}
	}

	return dialog;
}
