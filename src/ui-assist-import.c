/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2017 Maxime DOYEN
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

#include "hb-import.h"
#include "ui-assist-import.h"

#include "list_account.h"
#include "list_operation.h"

#include "ui-account.h"
#include "dsp_mainwindow.h"



/****************************************************************************/
/* Debug macros																*/
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

#define FORCE_SIZE 1
#define HEAD_IMAGE 0
#define SIDE_IMAGE 0


/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;


static gchar *page_titles[] =
{
	N_("Welcome"),
	N_("Select file"),
	N_("Import"),
	N_("Properties"),
	N_("Account"),
	N_("Transaction"),
	N_("Confirmation")
};


extern gchar *CYA_IMPORT_DATEORDER[];


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* account affect listview */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static guint32
ui_acc_affect_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Account *item;

		gtk_tree_model_get(model, &iter, 0, &item, -1);

		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}


static void
ui_acc_affect_listview_srcname_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;
gchar *string;

	gtk_tree_model_get(model, iter, 0, &entry, -1);

	name = entry->imp_name;

	#if MYDEBUG
		string = g_markup_printf_escaped("<i>[%d] %s</i>", entry->key, name );
	#else
		string = g_markup_printf_escaped("<i>%s</i>", name);
	#endif
	g_object_set(renderer, "markup", string, NULL);
	g_free(string);
}

static void
ui_acc_affect_listview_new_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;

	gtk_tree_model_get(model, iter, 0, &entry, -1);
	name = NULL;
	if(entry->imp_key == 0)
		name = _("create new");
	else
		name = _("use existing");

	g_object_set(renderer, "markup", name, NULL);

}

static void
ui_acc_affect_listview_dstname_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry, *dst_entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, 0, &entry, -1);
	name = NULL;
	if(entry->imp_key == 0)
		name = entry->name;
	else
	{
		dst_entry = da_acc_get(entry->imp_key);
		if( dst_entry != NULL )
			name = dst_entry->name;
	}

	#if MYDEBUG
		string = g_strdup_printf ("[%d] %s", entry->imp_key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}

static void
ui_acc_affect_listview_add(GtkTreeView *treeview, Account *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, item,
			-1);

		//gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}


static GtkWidget *
ui_acc_affect_listview_new(void)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(1,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	// column: import account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("Name in the file"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_srcname_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column: target account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("Action"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_new_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column: target account
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_title(column, _("Name in HomeBank"));
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_affect_listview_dstname_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);



	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), TRUE);

	//gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_listview_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

//old stuf for transition waiting import rewrite
static void da_obsolete_transaction_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Transaction *item = tmplist->data;
		da_transaction_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}

static GQueue *da_obsolete_transaction_get_partial(guint32 minjulian)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GQueue *txn_queue;

	txn_queue = g_queue_new ();

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		lnk_txn = g_queue_peek_tail_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;

			if( txn->date < minjulian ) //no need to go below mindate
				break;

			g_queue_push_head (txn_queue, txn);
			
			lnk_txn = g_list_previous(lnk_txn);
		}
	
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	return txn_queue;
}




/* count account to be imported */
static void _import_context_count(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
GList *lacc, *list;

	DB( g_print("\n[import] context count\n") );

	ictx->nb_src_acc = ictx->nb_new_acc = 0;

	ictx->cnt_new_ope = 0;

	/* count account */
	lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			ictx->nb_src_acc++;
			if( item->imp_key == 0 )
				ictx->nb_new_acc++;
		}
		list = g_list_next(list);
	}
	g_list_free(lacc);

	/* count transaction */
	ictx->cnt_new_ope = g_list_length(ictx->trans_list);

}


static void _import_context_clear(ImportContext *ictx)
{
	DB( g_print("\n[import] context clear\n") );

	if(ictx->trans_list)
		da_obsolete_transaction_destroy(ictx->trans_list);
	ictx->trans_list  = NULL;
	ictx->next_acc_key = da_acc_length();
	ictx->datefmt  = PREFS->dtex_datefmt;
	ictx->encoding = NULL;

	ictx->cnt_err_date  = 0;
	ictx->cnt_new_pay = 0;
	ictx->cnt_new_cat = 0;
}


#if MYDEBUG
static void _import_context_debug(ImportContext *ictx)
{
	DB( g_print("\n[import] context debug\n") );

	DB( g_print(
	    " -> txnlist=%p, maxacckey=%d\n"
	    " -> nb-acc=%d, nb-newacc=%d\n"
	    " -> ntxn=%d, npay=%d, ncat=%d\n"
		" -> datefmt=%d, encoding='%s', errdate=%d, ndup=%d\n",
	    ictx->trans_list, ictx->next_acc_key,
		ictx->nb_src_acc, ictx->nb_new_acc,
		ictx->cnt_new_ope,
		ictx->cnt_new_pay,
		ictx->cnt_new_cat,
		ictx->datefmt,
		ictx->encoding,
		ictx->cnt_err_date,
		ictx->nb_duplicate
		)
	   );
}
#endif







static GList *homebank_qif_import(gchar *filename, ImportContext *ictx)
{
GList *list = NULL;

	DB( g_print("\n[import] homebank QIF\n") );

	//todo: context ?
	list = account_import_qif(filename, ictx);

	return list;
}




static void import_clearall(struct import_data *data)
{
GList *lxxx, *list;
GtkTreeModel *model;

	DB( g_print("\n[import] clear all\n") );

	// clear account & transactions
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->duplicat_ope));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	
	// 1: delete imported accounts
	lxxx = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> delete acc %p '%s'\n", item, item->name) );
			da_acc_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	// 2: delete imported payees
	lxxx = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> delete pay '%s'\n", item->name) );
			da_pay_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	// 3: delete imported category
	lxxx = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if( item->imported == TRUE )
		{
			DB( g_print(" -> delete cat '%s'\n", item->name) );
			da_cat_remove(item->key);
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	_import_context_clear(&data->ictx);

}


static gboolean ui_import_panel_transaction_is_duplicate(Transaction *impope, Transaction *ope, gint maxgap)
{
Account *dstacc;
guint dstkacc;
gboolean retval = FALSE;
		
	//common tests
	if( (impope->amount == ope->amount) &&
		(ope->date <= (impope->date + maxgap)) && (ope->date >= (impope->date - maxgap)) )
	{

		//we focus the test on impope->acc->imp_key (and not impope->kacc)
		dstkacc = impope->kacc; 
		dstacc = da_acc_get(dstkacc);
		if( dstacc && dstacc->imp_key > 0 )
		{
			dstkacc = dstacc->imp_key;
		}

		DB( g_print("--------\n -> dstkacc=%d, amount & date are similar\n", dstkacc) );

		DB( g_print(" -> impope: kacc=%d, %s kxfer=%d, kxferacc=%d\n", impope->kacc, impope->wording, impope->kxfer, impope->kxferacc) );
		DB( g_print(" ->    ope: kacc=%d, %s kxfer=%d, kxferacc=%d\n", ope->kacc, ope->wording, ope->kxfer, ope->kxferacc) );


		if(impope->paymode != PAYMODE_INTXFER)
		{
			if( dstkacc == ope->kacc )
			{
				DB( g_print(" -> impope is not a xfer and acc are similar\n") );
				retval = TRUE;
			}
		}
		else
		{
			if( ( (impope->kxferacc == ope->kxferacc) && ope->kxfer != 0) ||
				( impope->kxferacc == 0 )
			   )
				retval = TRUE;
		}
	}
	return retval;
}


static void ui_import_panel_transaction_find_duplicate(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
GList *tmplist, *implist;
Transaction *item;
guint32 mindate;
guint maxgap;

	DB( g_print("\n[import] find duplicate\n") );

	ictx->nb_duplicate = 0;
	if( ictx->trans_list )
	{
		/* 1: get import min bound date */
		tmplist = g_list_first(ictx->trans_list);
		item = tmplist->data;
		mindate = item->date;
		maxgap = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_maxgap));

		/* clear any previous same txn */
		implist = g_list_first(ictx->trans_list);
		while (implist != NULL)
		{
		Transaction *impope = implist->data;

			if(impope->same != NULL)
			{
				g_list_free(impope->same);
				impope->same = NULL;
			}
			implist = g_list_next(implist);
		}
		
		
		GQueue *txn_queue = da_obsolete_transaction_get_partial(mindate);

		//tmplist = g_list_first(GLOBALS->ope_list);
		tmplist = g_queue_peek_head_link(txn_queue);
		while (tmplist != NULL)
		{
		Transaction *ope = tmplist->data;

			if( ope->date >= mindate )
			{
				//DB( g_print("should check here %d: %s\n", ope->date, ope->wording) );

				implist = g_list_first(ictx->trans_list);
				while (implist != NULL)
				{
				Transaction *impope = implist->data;

					if( ui_import_panel_transaction_is_duplicate(impope, ope, maxgap) )
					{
						//DB( g_print(" found %d: %s\n", impope->date, impope->wording) );

						impope->same = g_list_append(impope->same, ope);
						ictx->nb_duplicate++;
					}

					implist = g_list_next(implist);
				}
			}

			tmplist = g_list_next(tmplist);
		}
		
		g_queue_free (txn_queue);
		
	}

	DB( g_print(" nb_duplicate = %d\n", ictx->nb_duplicate) );


}


static void ui_import_panel_account_fill(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
gchar *label = NULL;
gchar *icon_name = NULL;
GList *lacc, *list;
	
	DB( g_print("\n[import] panel account fill\n") );

	if(ictx->nb_new_acc == 0)
	{
		icon_name = ICONNAME_INFO;
		label = g_strdup( _("All seems all right here, your validation is optional!") );
	}
	else
	{
	gchar *tmpstr;

		/* file name & path */
		tmpstr = g_path_get_basename(data->filename);



		icon_name = ICONNAME_WARNING;
		label = g_strdup_printf(
			_("No account information has been found into the file '%s'.\n"
			  "Please select the appropriate action for account below."),
		    tmpstr);

		g_free(tmpstr);
	}

	gtk_label_set_text(GTK_LABEL(data->LB_acc), label);
	gtk_image_set_from_icon_name(GTK_IMAGE(data->IM_acc), icon_name, GTK_ICON_SIZE_BUTTON);

	g_free(label);

	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc))));

	lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			ui_acc_affect_listview_add(GTK_TREE_VIEW(data->LV_acc), item);
		}
		list = g_list_next(list);
	}
	g_list_free(lacc);

	DB( _import_context_debug(&data->ictx) );
}


/* count transaction with checkbox 'import'  */
static void import_count_changes(struct import_data *data)
{
GList *lacc, *list;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;

	DB( g_print("\n[import] count_final_changes\n") );

	data->imp_cnt_acc = 0;

	lacc = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE && item->imp_key != 0)
		{
			data->imp_cnt_acc++;
		}
		list = g_list_next(list);
	}
	g_list_free(lacc);


	// then import transactions
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));

	data->imp_cnt_trn = 0;

	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
			data->imp_cnt_trn++;

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}


static void import_apply(struct import_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GList *lxxx, *list;

	DB( g_print("\n[import] apply\n") );

	// 1: persist imported accounts
	lxxx = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		if( item->imported == TRUE )
		{
			//only persist user selected to new account
			if( item->imp_key == 0)
			{
				DB( g_print(" -> persist acc %x '%s' k=%d, ik=%d\n", item, item->name, item->key, item->imp_key) );
				item->imported = FALSE;
				g_free(item->imp_name);
				item->imp_name = NULL;
			}
			//else
			//DB( g_print(" -> keep exist acc %x '%s' k=%d, ik=%d\n", item, item->name, item->key, item->imp_key) );
			
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	// 2: persist imported payees
	lxxx = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> persist pay '%s'\n", item->name) );
			item->imported = FALSE;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	// 3: persist imported categories
	lxxx = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if( item->imported == TRUE )
		{
			//DB( g_print(" -> persist cat '%s'\n", item->name) );
			item->imported = FALSE;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	// 4: insert every transactions
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->imported_ope));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Transaction *item;
	gboolean toimport;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DSPOPE_DATAS, &item,
			LST_OPE_IMPTOGGLE, &toimport,
			-1);

		if(toimport == TRUE)
		{
		Account *acc;
			
			//DB(g_print("import %d to acc: %d\n", data->total, item->account)	);

			//todo: here also test imp_key on account and change the key into the transaction
			acc = da_acc_get(item->kacc);
			if( (acc != NULL) && (acc->imp_key > 0) )
			{
				item->kacc = acc->imp_key;
			}

			//#1653957 change also kxferacc
			if( item->paymode == PAYMODE_INTXFER )
			{
				acc = da_acc_get(item->kxferacc);
				if( (acc != NULL) && (acc->imp_key > 0) )
				{
					item->kxferacc = acc->imp_key;
				}
			}
			
			transaction_add(item, NULL, 0);
		}

		/* Make iter point to the next row in the list store */
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}


}

/*
**
*/
static gboolean
ui_import_assistant_dispose(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;

	DB( g_print("\n[import] dispose\n") );

#if MYDEBUG == 1
	gpointer data2 = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	g_print(" user_data=%08x to be free, data2=%x\n", (gint)user_data, (gint)data2);
#endif

	g_free( data->filename );

	import_clearall(data);




	// todo: optimize this
	if(data->imp_cnt_trn > 0)
	{
		GLOBALS->changes_count += data->imp_cnt_trn;

		//our global list has changed, so update the treeview
		ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_REFRESHALL));
	}


	g_free(user_data);


	//delete-event TRUE abort/FALSE destroy
	return FALSE;
}


static void ui_import_panel_transaction_fill(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
GtkWidget *view;
GtkTreeModel *model;
GtkTreeIter	iter;
GList *tmplist;
gchar *label = NULL;
gchar *icon_name = NULL;

	//DB( g_print("\n[import] fill imp operatoin\n") );

	if(ictx->nb_duplicate == 0)
	{
		icon_name = ICONNAME_INFO;
		label = _("All seems all right here, your validation is optional!");
	}
	else
	{
		icon_name = ICONNAME_WARNING;
		label = 
			_("Possible duplicate of existing transaction have been found, and disabled for import.\n"
			  "Please check and choose the ones that have to be imported.");
	}

	gtk_label_set_text(GTK_LABEL(data->LB_txn), label);
	gtk_image_set_from_icon_name(GTK_IMAGE(data->IM_txn), icon_name, GTK_ICON_SIZE_BUTTON);

	
	view = data->imported_ope;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	tmplist = g_list_first(ictx->trans_list);
	while (tmplist != NULL)
	{
	Transaction *item = tmplist->data;

		/* append to our treeview */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);

				//DB( g_print(" populate: %s\n", ope->ope_Word) );

				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, item,
				LST_OPE_IMPTOGGLE, item->same == NULL ? TRUE : FALSE,
				-1);

		//DB( g_print(" - fill: %d, %s %.2f %x\n", item->account, item->wording, item->amount, item->same) );

		tmplist = g_list_next(tmplist);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */

	g_object_unref(model);



}


static void ui_import_panel_account_change_action_toggled_cb(GtkRadioButton *radiobutton, gpointer user_data)
{
struct import_target_data *data;
gboolean new_account;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[import] account type toggle\n") );

	new_account = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0]));

	gtk_widget_set_sensitive(data->label1, new_account);
	gtk_widget_set_sensitive(data->getwidget1, new_account);

	gtk_widget_set_sensitive(data->label2, new_account^1);
	gtk_widget_set_sensitive(data->getwidget2, new_account^1);

}


static void ui_import_panel_account_change_action(GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
struct import_target_data ddata;
ImportContext *ictx;
GtkWidget *dialog, *content_area, *group_grid, *label ;
guint32 key;
gint row;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n[import] account_change_action\n") );

	ictx = &data->ictx;
	
	key = ui_acc_affect_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
	Account *item;

		item = da_acc_get( key );

		dialog = gtk_dialog_new_with_buttons (_("Change account action"),
						    GTK_WINDOW (data->assistant),
						    0,
						    _("_Cancel"),
						    GTK_RESPONSE_REJECT,
						    _("_OK"),
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		//store our window private data
		g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&ddata);

		content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
		
		// group :: dialog
		group_grid = gtk_grid_new ();
		gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
		gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
		gtk_container_set_border_width (GTK_CONTAINER(group_grid), SPACING_MEDIUM);
		gtk_box_pack_start (GTK_BOX (content_area), group_grid, TRUE, TRUE, SPACING_SMALL);

		row = 0;
		ddata.radio[0] = gtk_radio_button_new_with_label (NULL, _("create new"));
		gtk_grid_attach (GTK_GRID (group_grid), ddata.radio[0], 0, row, 3, 1);

			row++;
			label = make_label(_("_Name:"), 0, 0.5);
			ddata.label1 = label;
			gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

			ddata.getwidget1 = gtk_entry_new();
			gtk_grid_attach (GTK_GRID (group_grid), ddata.getwidget1, 2, row, 1, 1);

		row++;
		ddata.radio[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (ddata.radio[0]), _("use existing"));
		gtk_grid_attach (GTK_GRID (group_grid), ddata.radio[1], 0, row, 3, 1);
		
			row++;
			label = make_label(_("A_ccount:"), 0, 0.5);
			ddata.label2 = label;
			gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);

			ddata.getwidget2 = ui_acc_comboboxentry_new(NULL);
			gtk_grid_attach (GTK_GRID (group_grid), ddata.getwidget2, 2, row, 1, 1);

	//initialize
		if( ictx->next_acc_key > 0 )	//if there were already some accounts
		{
			gtk_widget_set_sensitive(ddata.radio[1], TRUE);
			if( item->imp_key > 0 )
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ddata.radio[1]), TRUE);
			}
		}
		else
		{
			gtk_widget_set_sensitive(ddata.radio[1], FALSE);

		}

		gtk_entry_set_text(GTK_ENTRY(ddata.getwidget1), item->name);
		ui_acc_comboboxentry_populate(GTK_COMBO_BOX(ddata.getwidget2), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(ddata.getwidget2), item->imp_key);

		ui_import_panel_account_change_action_toggled_cb(GTK_RADIO_BUTTON (ddata.radio[0]), NULL);

		gtk_widget_show_all(group_grid);

		g_signal_connect (ddata.radio[0], "toggled", G_CALLBACK (ui_import_panel_account_change_action_toggled_cb), NULL);


		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gchar *name;
		gboolean bnew;
		guint key;

			key = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(ddata.getwidget2));

			bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ddata.radio[0]));
			if( bnew )
			{

				name = (gchar *)gtk_entry_get_text(GTK_ENTRY(ddata.getwidget1));

				if(strcasecmp(name, item->name))
				{

					DB( g_print("name '%s', existing acc %d\n", name, key) );

					if (name && *name)
					{
						if( account_rename(item, name) == FALSE )
						{
							ui_dialog_msg_infoerror(GTK_WINDOW(dialog), GTK_MESSAGE_ERROR,
								_("Error"),
								_("Cannot rename this Account,\n"
								"from '%s' to '%s',\n"
								"this name already exists."),
								item->name,
								name
								);
						}
					}
				}
				else
				{
					item->imp_key = 0;
				}
			}
			else
			{
				item->imp_key = key;
			}

			//we should refresh duplicate
			ui_import_panel_transaction_find_duplicate(data);
			ui_import_panel_transaction_fill(data);

	    }

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	}

}


static void ui_import_panel_filechooser_selection_changed(GtkWidget *widget, gpointer user_data)
{
struct import_data *data = user_data;
gint page_number;
GtkWidget *current_page;
gchar *filename;

	page_number = gtk_assistant_get_current_page (GTK_ASSISTANT(data->assistant));

	DB( g_print("\n[import] selchange (page %d)\n", page_number+1) );

	data->valid = FALSE;

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(data->filechooser));
	if( filename == NULL )
	{
		gtk_label_set_text(GTK_LABEL(data->user_info), _("Please select a file..."));
		//current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);
		//gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, FALSE);
	}
	else
	{
		if( page_number == PAGE_SELECTFILE )
		{
			if(data->filename)
				g_free( data->filename );
			data->filename = filename;
			//DB( g_print(" filename -> %s\n", data->filename) );

			data->filetype = homebank_alienfile_recognize(data->filename);
			switch(data->filetype)
			{
				case FILETYPE_QIF:
					gtk_label_set_text(GTK_LABEL(data->user_info), _("QIF file recognised !"));
					data->valid = TRUE;
					break;

				case FILETYPE_OFX:
					#ifndef NOOFX
					gtk_label_set_text(GTK_LABEL(data->user_info), _("OFX file recognised !"));
					data->valid = TRUE;
					#else
					gtk_label_set_text(GTK_LABEL(data->user_info), _("** OFX support is disabled **"));
					#endif
					break;

				case FILETYPE_CSV_HB:
					gtk_label_set_text(GTK_LABEL(data->user_info), _("CSV transaction file recognised !"));
					data->valid = TRUE;
					break;

				default:
					data->filetype = FILETYPE_UNKNOW;
					gtk_label_set_text(GTK_LABEL(data->user_info), _("Unknown/Invalid file..."));
					break;
			}

			current_page = gtk_assistant_get_nth_page (GTK_ASSISTANT(data->assistant), page_number);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), current_page, data->valid);

		}

	}

	if(data->valid == TRUE)
	{
		gtk_widget_show(data->ok_image);
		gtk_widget_hide(data->ko_image);
	}
	else
	{
		gtk_widget_show(data->ko_image);
		gtk_widget_hide(data->ok_image);
	}

}



static void ui_import_panel_transaction_fill_same(GtkTreeSelection *treeselection, gpointer user_data)
{
struct import_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model, *newmodel;
GtkTreeIter			 iter, newiter;
GList *tmplist;
GtkWidget *view, *widget;

	widget = GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection));


	//DB( g_print("\n[import] fillsame\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	view = data->duplicat_ope;

	newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(newmodel));



	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Transaction *item;

		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &item, -1);

		if( item->same != NULL )
		{
			tmplist = g_list_first(item->same);
			while (tmplist != NULL)
			{
			Transaction *tmp = tmplist->data;

				/* append to our treeview */
					gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

					gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
					LST_DSPOPE_DATAS, tmp,
					-1);

				//DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, (unsigned int)item->same) );

				tmplist = g_list_next(tmplist);
			}
		}

	}

	


}


static void ui_import_panel_properties_fill(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
gchar *tmpstr;

	/* file name & path */
	tmpstr = g_path_get_basename(data->filename);
	gtk_label_set_text(GTK_LABEL(data->TX_filename), tmpstr);
	g_free(tmpstr);
	
	tmpstr = g_path_get_dirname(data->filename);
	gtk_label_set_text(GTK_LABEL(data->TX_filepath), tmpstr);
	g_free(tmpstr);
	
	gtk_label_set_text(GTK_LABEL(data->TX_encoding), ictx->encoding);
	
	gtk_label_set_text(GTK_LABEL(data->TX_datefmt), CYA_IMPORT_DATEORDER[ictx->datefmt]);

	/* file content detail */
	//TODO: difficult translation here
	tmpstr = g_strdup_printf(_("account: %d - transaction: %d - payee: %d - categorie: %d"),
				ictx->nb_src_acc,
				ictx->cnt_new_ope,
				ictx->cnt_new_pay,
				ictx->cnt_new_cat
				);
	gtk_label_set_text(GTK_LABEL(data->TX_filedetails), tmpstr);
	g_free(tmpstr);

	DB( _import_context_debug(&data->ictx) );
	
}


static void ui_import_panel_confirmation_fill(struct import_data *data)
{
ImportContext *ictx = &data->ictx;
	
	/* account summary */
	ui_label_set_integer(GTK_LABEL(data->TX_acc_upd), data->imp_cnt_acc);
	ui_label_set_integer(GTK_LABEL(data->TX_acc_new), ictx->nb_src_acc - data->imp_cnt_acc);

	/* transaction summary */
	ui_label_set_integer(GTK_LABEL(data->TX_trn_imp), data->imp_cnt_trn);
	ui_label_set_integer(GTK_LABEL(data->TX_trn_nop), ictx->cnt_new_ope - data->imp_cnt_trn);
	ui_label_set_integer(GTK_LABEL(data->TX_trn_asg), data->imp_cnt_asg);

}

static void
ui_import_assistant_apply (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;

	DB( g_print("\n[import] apply\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	import_apply(data);

}

static void
ui_import_assistant_close_cancel (GtkWidget *widget, gpointer user_data)
{
struct import_data *data;
	GtkWidget *assistant = (GtkWidget *) user_data;

	DB( g_print("\n[import] close\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	ui_import_assistant_dispose(widget, data);


	//g_free(data);


	gtk_widget_destroy (assistant);
	//assistant = NULL;
}


static void _import_tryload_file(struct import_data *data)
{
ImportContext *ictx = &data->ictx;

	DB( g_print("\n[import] try load file\n") );

	DB( g_print(" -> encoding='%s'\n", ictx->encoding) );
	DB( g_print(" -> date format='%s' (%d)\n", CYA_IMPORT_DATEORDER[ictx->datefmt], ictx->datefmt) );

	
	switch(data->filetype)
	{
#ifndef NOOFX
		/* ofx_acc_list & ofx_ope_list are filled here */
		case FILETYPE_OFX:
			ictx->trans_list = homebank_ofx_import(data->filename, &data->ictx);
			break;
#endif
		case FILETYPE_QIF:
			ictx->trans_list = homebank_qif_import(data->filename, &data->ictx);
			break;

		case FILETYPE_CSV_HB:
			ictx->trans_list = homebank_csv_import(data->filename, &data->ictx);
			break;
	}

	DB( g_print(" -> result: nbtrans=%d, date errors=%d\n", ictx->cnt_new_ope, ictx->cnt_err_date) );

	
}


static void import_file_import(struct import_data *data)
{
ImportContext *ictx = &data->ictx;

	DB( g_print("\n[import] real import\n") );

	import_clearall(data);
	ictx->encoding = homebank_file_getencoding(data->filename);
	_import_tryload_file(data);

	// if fail, try to load with different date format
	if( ictx->cnt_err_date > 0)
	{
	const gchar *encoding = ictx->encoding;
	gint i;

		for(i=0;i<NUM_PRF_DATEFMT;i++)
		{
		gboolean do_fix;

			if(i != PREFS->dtex_datefmt)	//don't reload with user pref date format
			{
				do_fix = ui_dialog_msg_question(
					GTK_WINDOW(data->assistant),
					_("Some date convertion failed"),
					_("Reload using date order: '%s' ?"),
					CYA_IMPORT_DATEORDER[i]
					);

				if(do_fix == GTK_RESPONSE_YES)
				{	
					DB( g_print(" fail, reload with '%s'\n", CYA_IMPORT_DATEORDER[i]) );
					//#1448549
					import_clearall(data);
					ictx->encoding = encoding; //#1425986 keep encoding with us
					ictx->datefmt = i;
					_import_tryload_file(data);

					DB( g_print(" -> reloaded: nbtrans=%d, date errors=%d\n", ictx->cnt_new_ope, ictx->cnt_err_date) );

					if(ictx->cnt_err_date == 0)
						break;
				}

			}
		}
		
	}

	DB( g_print(" end of try import\n") );

	// sort by date
	ictx->trans_list = da_transaction_sort(ictx->trans_list);

}

/**
 * ui_import_assistant_forward_page_func:
 *
 * define the page to be called when the user forward
 *
 * Return value: the page number
 *
 */
static gint
ui_import_assistant_forward_page_func(gint current_page, gpointer func_data)
{
gint next_page;

	DB( g_print("---------------------------\n") );
	DB( g_print("\n[import] forward page func :: page %d\n", current_page) );

	DB( g_print(" -> current: %d %s\n", current_page, page_titles[MIN(current_page, NUM_PAGE-1)] ) );

#ifdef MYDEBUG
	/*
	struct import_data *data = func_data;
	gint i
	for(i=0;i<NUM_PAGE;i++)
	{
		g_print("%d: %d '%s'\n", i, 
		        gtk_assistant_get_page_complete(GTK_ASSISTANT(data->assistant), data->pages[i]),
		        page_titles[i]
				);
	}*/
#endif
	
	DB( g_print(" -> current: %d %s\n", current_page, page_titles[MIN(current_page, NUM_PAGE-1)] ) );
	
	next_page = current_page + 1;	
	
	switch(current_page)
	{
		/*case PAGE_IMPORT:
			// if no new account, skip the account page
			if(ictx->nb_new_acc == 0)
				next_page = PAGE_TRANSACTION;
			break;*/
	}

	DB( g_print(" -> next: %d %s\n", next_page, page_titles[MIN(next_page, NUM_PAGE-1)]  ) );

	return next_page;
}




static void
ui_import_assistant_prepare (GtkWidget *widget, GtkWidget *page, gpointer user_data)
{
struct import_data *data;
ImportContext *ictx;
gint current_page, n_pages;
gchar *title;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	ictx = &data->ictx;
	
	current_page = gtk_assistant_get_current_page (GTK_ASSISTANT(data->assistant));
	n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT(data->assistant));

	DB( g_print("\n[import] prepare %d of %d\n", current_page, n_pages) );

	switch( current_page  )
	{
		case PAGE_WELCOME:
			DB( g_print(" -> 1 intro\n") );
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;

		case PAGE_SELECTFILE:
			DB( g_print(" -> 2 file choose\n") );
		
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, FALSE);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_ACCOUNT], FALSE);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_TRANSACTION], FALSE);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_CONFIRM], FALSE);

			gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(data->filechooser), PREFS->path_import);
			DB( g_print(" -> set current folder '%s'\n", PREFS->path_import) );

			// the page complete is contextual in ui_import_panel_filechooser_selection_changed
			break;

		case PAGE_IMPORT:
			DB( g_print(" -> 3 real import\n") );

			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, FALSE);
			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_ACCOUNT], FALSE);

			/* remind folder to preference */
			gchar *folder = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(data->filechooser));
			DB( g_print(" -> store folder '%s'\n", folder) );
			g_free(PREFS->path_import);
			PREFS->path_import = folder;

			import_file_import(data);
			_import_context_count(data);
			
			if( ictx->cnt_new_ope > 0 && ictx->cnt_err_date <= 0 )
			{   
					if(ictx->nb_new_acc == 0)
					{
						DB( g_print(" -> jump to Transaction page\n") );
						//gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), data->pages[PAGE_ACCOUNT], TRUE);
						gtk_assistant_next_page(GTK_ASSISTANT(data->assistant));
						gtk_assistant_next_page(GTK_ASSISTANT(data->assistant));
						gtk_assistant_next_page(GTK_ASSISTANT(data->assistant));
						//gtk_assistant_set_current_page (GTK_ASSISTANT(data->assistant), PAGE_TRANSACTION);
					}
					else
					{
						DB( g_print(" -> jump to Account page\n") );
						//gtk_assistant_set_current_page (GTK_ASSISTANT(data->assistant), PAGE_ACCOUNT);
						gtk_assistant_next_page(GTK_ASSISTANT(data->assistant));
						gtk_assistant_next_page(GTK_ASSISTANT(data->assistant));
					}

				gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			}
			break;		

		case PAGE_PROPERTIES:
			DB( g_print(" -> 4 properties\n") );
			

			ui_import_panel_properties_fill(data);

			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
			
		case PAGE_ACCOUNT:
			DB( g_print(" -> 5 account\n") );
	
			ui_import_panel_account_fill(data);

			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;

		case PAGE_TRANSACTION:
			DB( g_print(" -> 6 transaction\n") );

			//todo: should be optional
			data->imp_cnt_asg = transaction_auto_assign(ictx->trans_list, 0);

			ui_import_panel_transaction_find_duplicate(data);

			ui_import_panel_transaction_fill(data);

			if( ictx->nb_duplicate > 0 )
			{
				gtk_widget_show(data->GR_duplicate);
				gtk_expander_set_expanded(GTK_EXPANDER(data->GR_duplicate), TRUE);
			}
			else
			{
				gtk_widget_hide(data->GR_duplicate);
			}

			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
			
		case PAGE_CONFIRM:
		{
			DB( g_print(" -> 7 confirmation\n") );

			//todo:rework this
			import_count_changes(data);

			ui_import_panel_confirmation_fill(data);



 			gtk_assistant_set_page_complete (GTK_ASSISTANT(data->assistant), page, TRUE);
			break;
		}
	}

	title = g_strdup_printf ( _("Import assistant (%d of %d)"), current_page + 1 , n_pages );
	gtk_window_set_title (GTK_WINDOW (data->assistant), title);
	g_free (title);
}






static void
ui_import_panel_transaction_refresh (GtkWidget *widget, gpointer data)
{

	DB( g_print("\n[import] refresh transaction\n") );

	ui_import_panel_transaction_find_duplicate(data);
	ui_import_panel_transaction_fill(data);

}


static void ui_acc_affect_listview_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
//GtkTreeModel		 *model;

	//model = gtk_tree_view_get_model(treeview);
	//gtk_tree_model_get_iter_first(model, &iter);
	//if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	//{
		ui_import_panel_account_change_action(GTK_WIDGET(treeview), NULL);
	//}
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static GtkWidget *
ui_import_panel_welcome_create(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label, *align;

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	//gtk_container_set_border_width (GTK_CONTAINER(vbox), SPACING_MEDIUM);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	label = make_label(
	    _("Welcome to the HomeBank Import Assistant.\n\n" \
		"With this assistant you will be guided throught the process\n" \
		"of importing an external file into HomeBank.\n\n" \
	    "No changes will be made until you click \"Apply\" at the end\n" \
	    "of this assistant.")
			, 0., 0.0);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, SPACING_SMALL);

	/* supported format */


	label = make_label(
	    _("HomeBank can import files in the following formats:\n" \
		"- QIF\n" \
		"- OFX/QFX (optional at compilation time)\n" \
		"- CSV (format is specific to HomeBank, see the documentation)\n" \
	), 0.0, 0.0);

	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, SPACING_SMALL);


	gtk_widget_show_all (align);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), align);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), align, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), align, _(page_titles[PAGE_WELCOME]));
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), align, TRUE);

	return align;
}


static GtkWidget *
ui_import_panel_filechooser_create (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *hbox, *align, *widget, *label;
GtkFileFilter *filter;

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	//gtk_container_set_border_width (GTK_CONTAINER(vbox), SPACING_MEDIUM);


//	widget = gtk_file_chooser_button_new ("Pick a File", GTK_FILE_CHOOSER_ACTION_OPEN);

	widget = gtk_file_chooser_widget_new(GTK_FILE_CHOOSER_ACTION_OPEN);


	
	data->filechooser = widget;
	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Known files"));
	gtk_file_filter_add_pattern (filter, "*.[Qq][Ii][Ff]");
	#ifndef NOOFX
	gtk_file_filter_add_pattern (filter, "*.[OoQq][Ff][Xx]");
	#endif
	gtk_file_filter_add_pattern (filter, "*.[Cc][Ss][Vv]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);
	if(data->filetype == FILETYPE_UNKNOW)
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(widget), filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("QIF files"));
	gtk_file_filter_add_pattern (filter, "*.[Qq][Ii][Ff]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);
	if(data->filetype == FILETYPE_QIF)
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(widget), filter);
	
	#ifndef NOOFX
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("OFX/QFX files"));
	gtk_file_filter_add_pattern (filter, "*.[OoQq][Ff][Xx]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);
	if(data->filetype == FILETYPE_OFX)
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(widget), filter);
	#endif

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("CSV files"));
	gtk_file_filter_add_pattern (filter, "*.[Cc][Ss][Vv]");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);
	if(data->filetype == FILETYPE_CSV_HB)
		gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(widget), filter);

	
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
	gtk_file_filter_add_pattern (filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(widget), filter);


/* our addon message */
	align = gtk_alignment_new(0.65, 0, 0, 0);
		gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 0);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_container_add(GTK_CONTAINER(align), hbox);

	label = gtk_label_new("");
	data->user_info = label;
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, SPACING_SMALL);

	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE,
							PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD,
                             -1);



	widget = gtk_image_new_from_icon_name(ICONNAME_HB_FILE_VALID, GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->ok_image = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	widget = gtk_image_new_from_icon_name(ICONNAME_HB_FILE_INVALID, GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->ko_image = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	gtk_widget_show_all (vbox);
	gtk_widget_hide(data->ok_image);
	gtk_widget_hide(data->ko_image);


	gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
	//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, GTK_ASSISTANT_PAGE_CONTENT);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, _(page_titles[PAGE_SELECTFILE]));



	return vbox;
}


static GtkWidget *
ui_import_panel_import_create (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *align, *content_grid;
GtkWidget *label, *widget;
gchar *txt;

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	
	content_grid = gtk_grid_new();
	gtk_grid_set_column_spacing (GTK_GRID (content_grid), SPACING_MEDIUM);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_add(GTK_CONTAINER(align), content_grid);
	
	widget = gtk_image_new_from_icon_name(ICONNAME_ERROR, GTK_ICON_SIZE_DIALOG );
	gtk_grid_attach (GTK_GRID (content_grid), widget, 0, 0, 1, 1);
	
	txt = _("A general error occured, and this file cannot be loaded.");
	label = gtk_label_new(txt);
	gtk_widget_set_valign (label, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (content_grid), label, 1, 0, 1, 1);
	
	gtk_widget_show_all (align);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), align);
	//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), align, GTK_ASSISTANT_PAGE_PROGRESS);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), align, _(page_titles[PAGE_IMPORT]));
	
	return align;
}



static GtkWidget *
ui_import_panel_properties_create (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *content_grid, *group_grid;
GtkWidget *label, *widget;
gint crow, row;
	
	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: File properties
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("File properties"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label(_("Name:"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filename = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label(_("Path:"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filepath = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label(_("Encoding:"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_encoding = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label(_("Date format:"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_datefmt = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: File content
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("File content"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label(_("Content:"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_filedetails = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	/*
	expander = gtk_expander_new (_("File content"));
	gtk_box_pack_start (GTK_BOX (container), expander, TRUE, TRUE, 0);
 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	widget = gtk_text_view_new ();
	gtk_container_add(GTK_CONTAINER(scrollwin), widget);
	gtk_container_add(GTK_CONTAINER(expander), scrollwin);
	*/


	gtk_widget_show_all (content_grid);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), content_grid);
	//set page type to intro to avoid going back once that point over
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), content_grid, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), content_grid, _(page_titles[PAGE_PROPERTIES]));

	return content_grid;
}


static GtkWidget *
ui_import_panel_account_create (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *content_grid, *group_grid;
GtkWidget *label, *widget, *scrollwin;
gint crow, row;
	
	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Title
    group_grid = gtk_grid_new ();
	
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	//label = make_label_group(_("Title"));
	//gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 2, 1);
	
	row = 1;
	widget = gtk_image_new ();
	data->IM_acc = widget;
	gtk_widget_set_valign(widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 0, row, 1, 1);
	label = make_label(NULL, 0, 0.5);
	data->LB_acc = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 2, 1);

	// group :: Account list
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Choose the action for accounts"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 1, 1);
	
	row = 1;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_widget_set_size_request(scrollwin, -1, HB_MINWIDTH_LIST);

	widget = ui_acc_affect_listview_new();
	data->LV_acc = widget;
	gtk_container_add(GTK_CONTAINER(scrollwin), widget);
	gtk_widget_set_hexpand(scrollwin, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), scrollwin, 0, row, 1, 1);

	row++;
	widget = gtk_button_new_with_mnemonic (_("Change _action"));
	data->BT_edit = widget;
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 0, row, 1, 1);

	/* signal and other stuff */
	g_signal_connect (G_OBJECT (data->BT_edit), "clicked", G_CALLBACK (ui_import_panel_account_change_action), data);
	g_signal_connect (GTK_TREE_VIEW(data->LV_acc), "row-activated", G_CALLBACK (ui_acc_affect_listview_onRowActivated), NULL);

	gtk_widget_show_all (content_grid);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), content_grid);
	//gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), content_grid, GTK_ASSISTANT_PAGE_INTRO);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), content_grid, _(page_titles[PAGE_ACCOUNT]));
	
	return content_grid;
}


static GtkWidget *
ui_import_panel_transaction_create (GtkWidget *assistant, struct import_data *data)
{
GtkWidget *content_grid, *group_grid;
GtkWidget *label, *scrollwin, *widget, *expander;
gint crow, row;
	
	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);

	crow = 0;
	// group :: Title
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	//label = make_label_group(_("Title"));
	//gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	widget = gtk_image_new ();
	data->IM_txn = widget;
	gtk_widget_set_valign(widget, GTK_ALIGN_START);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 0, row, 1, 1);
	label = make_label(NULL, 0, 0.5);
	data->LB_txn = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 2, 1);

	// group :: Transactions to import
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Choose transactions to import"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 2, 1);
	
	row = 1;
	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_hexpand(scrollwin, TRUE);
	gtk_widget_set_vexpand(scrollwin, TRUE);
	widget = create_list_import_transaction(TRUE);
	data->imported_ope = widget;
	gtk_container_add (GTK_CONTAINER (scrollwin), widget);
	gtk_grid_attach (GTK_GRID (group_grid), scrollwin, 0, row, 2, 1);

	expander = gtk_expander_new (_("Detail of existing transaction (possible duplicate)"));
	data->GR_duplicate = expander;
	gtk_grid_attach (GTK_GRID (group_grid), expander, 0, crow++, 2, 1);

    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (expander), group_grid);

	row = 0;
	scrollwin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_hexpand(scrollwin, TRUE);
	//widget = create_list_transaction(LIST_TXN_TYPE_IMPORT, list_imptxn_columns);
	widget = create_list_import_transaction(FALSE);
	data->duplicat_ope = widget;
	gtk_container_add (GTK_CONTAINER (scrollwin), widget);
	gtk_widget_set_size_request(scrollwin, -1, HB_MINWIDTH_LIST/2);
	gtk_grid_attach (GTK_GRID (group_grid), scrollwin, 0, row, 6, 1);

	row++;
	label = make_label(_("Date _tolerance:"), 0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);

	widget = make_numeric(label, 0.0, 14.0);
	data->NB_maxgap = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	//TRANSLATORS: there is a spinner on the left of this label, and so you have 0....x days of date tolerance
	label = make_label(_("days"), 0, 0.5);
	gtk_grid_attach (GTK_GRID (group_grid), label, 2, row, 1, 1);

	widget = gtk_button_new_with_mnemonic (_("_Refresh"));
	gtk_grid_attach (GTK_GRID (group_grid), widget, 3, row, 1, 1);
	g_signal_connect (widget, "clicked",
			G_CALLBACK (ui_import_panel_transaction_refresh), data);

	widget = gtk_image_new_from_icon_name(ICONNAME_INFO, GTK_ICON_SIZE_SMALL_TOOLBAR );
	gtk_widget_set_hexpand(widget, FALSE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 4, row, 1, 1);

	label = make_label (_(
		"The match is done in order: by account, amount and date.\n" \
		"A date tolerance of 0 day means an exact match"), 0, 0.5);
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
	gtk_widget_set_hexpand(label, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), label, 5, row, 1, 1);


	gtk_widget_show_all (content_grid);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), content_grid);
//	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), content_grid, GTK_ASSISTANT_PAGE_PROGRESS);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), content_grid, _(page_titles[PAGE_TRANSACTION]));
	
	return content_grid;
}


static GtkWidget *
ui_import_panel_confirmation_create(GtkWidget *assistant, struct import_data *data)
{
GtkWidget *vbox, *label, *align, *widget, *table;
gint row;

	align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 0);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER(vbox), SPACING_MEDIUM);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	label = make_label(
		_("Click \"Apply\" to update your accounts.\n"), 0.5, 0.5);
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	/* the summary */
	table = gtk_grid_new ();
	gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_SMALL);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL/2);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	row = 0;
	label = make_label(_("Accounts"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	/* acc update */
	row++;
	label = make_label(NULL, 0.0, 0.5);
	//gtk_misc_set_padding (GTK_MISC (label), SPACING_SMALL, 0);
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_acc_upd = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
	label = make_label(_("to update"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);

	/* acc create */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_acc_new = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
	label = make_label(_("to create"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);

	row++;
	label = make_label(_("Transactions"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	/* trn import */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_imp = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
	label = make_label(_("to import"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);

	/* trn reject */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_nop = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
	label = make_label(_("to reject"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);

	/* trn auto-assigned */
	row++;
	widget = make_label(NULL, 1.0, 0.5);
	data->TX_trn_asg = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
	label = make_label(_("auto-assigned"), 0.0, 0.5);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);


	gtk_widget_show_all (align);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), align);
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), align, GTK_ASSISTANT_PAGE_CONFIRM);
	//gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), align, _(page_titles[PAGE_CONFIRM]));

	return align;
}


/* starting point of import */
GtkWidget *ui_import_assistant_new (gint filetype)
{
struct import_data *data;
GtkWidget *assistant;
gint w, h, pos;

	data = g_malloc0(sizeof(struct import_data));
	if(!data) return NULL;

	data->filetype = filetype;
	
	assistant = gtk_assistant_new ();
	data->assistant = assistant;

	//store our window private data
	g_object_set_data(G_OBJECT(assistant), "inst_data", (gpointer)data);
	//DB( g_print("** \n[import] window=%x, inst_data=%x\n", assistant, data) );


	gtk_window_set_modal(GTK_WINDOW (assistant), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(assistant), GTK_WINDOW(GLOBALS->mainwindow));

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(assistant), w*0.8, h*0.8);


	pos = 0;
	data->pages[pos++] = ui_import_panel_welcome_create (assistant, data);
	data->pages[pos++] = ui_import_panel_filechooser_create (assistant, data);
	data->pages[pos++] = ui_import_panel_import_create (assistant, data);
	data->pages[pos++] = ui_import_panel_properties_create (assistant, data);
	data->pages[pos++] = ui_import_panel_account_create (assistant, data);
	data->pages[pos++] = ui_import_panel_transaction_create (assistant, data);
	data->pages[pos++] = ui_import_panel_confirmation_create (assistant, data);

	gtk_assistant_set_forward_page_func(GTK_ASSISTANT(assistant), ui_import_assistant_forward_page_func, data, NULL);

	//setup

	ui_import_panel_filechooser_selection_changed(assistant, data);

	//connect all our signals
	//g_signal_connect (window, "delete-event", G_CALLBACK (hbfile_dispose), (gpointer)data);

	g_signal_connect (G_OBJECT (assistant), "cancel", G_CALLBACK (ui_import_assistant_close_cancel), assistant);

	g_signal_connect (G_OBJECT (assistant), "close", G_CALLBACK (ui_import_assistant_close_cancel), assistant);

	g_signal_connect (G_OBJECT (assistant), "apply", G_CALLBACK (ui_import_assistant_apply), NULL);

	g_signal_connect (G_OBJECT (assistant), "prepare", G_CALLBACK (ui_import_assistant_prepare), NULL);

	g_signal_connect (G_OBJECT (data->filechooser), "selection-changed",
		G_CALLBACK (ui_import_panel_filechooser_selection_changed), (gpointer)data);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->imported_ope)), "changed",
		G_CALLBACK (ui_import_panel_transaction_fill_same), NULL);

	gtk_widget_show (assistant);

	gtk_assistant_set_page_complete (GTK_ASSISTANT(assistant), data->pages[PAGE_WELCOME], TRUE );
	gtk_assistant_set_current_page(GTK_ASSISTANT(assistant), PAGE_SELECTFILE);

	return assistant;
}




