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


#include "homebank.h"

#include "hub-transaction.h"
#include "hub-account.h"
#include "dsp-mainwindow.h"
#include "list-operation.h"

#include "ui-transaction.h"


/****************************************************************************/
/* Debug macros                                                             */
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


void ui_hub_transaction_populate(struct hbfile_data *data)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GtkTreeModel *model1, *model2;
GtkTreeIter	iter;

	DB( g_print("\n[ui_hub_txn] populate\n") );

	model1 = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_txn[HUB_TXN_TYPE_FUTURE]));
	model2 = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_txn[HUB_TXN_TYPE_REMIND]));

	gtk_list_store_clear (GTK_LIST_STORE(model1));
	gtk_list_store_clear (GTK_LIST_STORE(model2));

	
	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		// skip closed in showall mode
		if( (acc->flags & AF_CLOSED) )
			goto next_acc;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;

			if(txn->date > GLOBALS->today)
			{
				gtk_list_store_insert_with_values(GTK_LIST_STORE(model1), &iter, -1,
						MODEL_TXN_POINTER, txn,
						-1);
			}

			if(txn->status == TXN_STATUS_REMIND)
			{
				gtk_list_store_insert_with_values(GTK_LIST_STORE(model2), &iter, -1,
						MODEL_TXN_POINTER, txn,
						-1);
			}
			lnk_txn = g_list_next(lnk_txn);
		}
	
	next_acc:
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);
}


static void ui_hub_transaction_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct hbfile_data *data = userdata;
Transaction *active_txn;
gboolean result;


	DB( g_print ("\n[ui_hub_txn] row double-clicked\n") );

	active_txn = list_txn_get_active_transaction(treeview);
	if(active_txn)
	{
	Transaction *old_txn, *new_txn;

		old_txn = da_transaction_clone (active_txn);
		new_txn = active_txn;
		result = deftransaction_external_edit(GTK_WINDOW(data->window), old_txn, new_txn);

		if(result == GTK_RESPONSE_ACCEPT)
		{
			//#1640885
			GLOBALS->changes_count++;
			ui_hub_transaction_populate(data);

			//#1824515 when amount change update acc panel
			if( old_txn->amount != new_txn->amount )
				ui_hub_account_populate(GLOBALS->mainwindow, NULL);
			
		}

		da_transaction_free (old_txn);
	}
}



GtkWidget *ui_hub_transaction_create(struct hbfile_data *data, HbHubTxnType type)
{
GtkWidget *hub, *vbox, *sw, *widget;

	DB( g_print("\n[ui_hub_txn] create %d\n", type) );
	
	if( type > HUB_TXN_TYPE_REMIND )
		return NULL;
	
	hub = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hub), SPACING_SMALL);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	//gtk_widget_set_margin_top(GTK_WIDGET(vbox), 0);
	//gtk_widget_set_margin_bottom(GTK_WIDGET(vbox), SPACING_SMALL);
	//gtk_widget_set_margin_start(GTK_WIDGET(vbox), 2*SPACING_SMALL);
	//gtk_widget_set_margin_end(GTK_WIDGET(vbox), SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (hub), vbox, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
	
	widget = (GtkWidget *)create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	list_txn_set_column_acc_visible(GTK_TREE_VIEW(widget), TRUE);
	data->LV_txn[type] = widget;
	gtk_container_add (GTK_CONTAINER (sw), widget);

	g_signal_connect (GTK_TREE_VIEW(data->LV_txn[type]), "row-activated", G_CALLBACK (ui_hub_transaction_onRowActivated), data);
	
	return hub;
}

