/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2014 Maxime DOYEN
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

#include "dsp_account.h"
#include "dsp_mainwindow.h"

#include "list_operation.h"
#include "ui-widgets.h"
#include "ui-filter.h"
#include "ui-transaction.h"
#include "gtk-dateentry.h"

#include "ui-payee.h"
#include "ui-category.h"


/****************************************************************************/
/* Debug macros																														 */
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

//debug
#define UI 1

//extern gchar *CYA_FLT_SELECT[];

extern gchar *CYA_FLT_TYPE[];
extern gchar *CYA_FLT_STATUS[];


static void register_panel_collect_filtered_txn(GtkWidget *view);
static void register_panel_listview_populate(GtkWidget *view);
static void register_panel_action(GtkWidget *widget, gpointer user_data);
static void register_panel_update(GtkWidget *widget, gpointer user_data);

static void register_panel_export_csv(GtkWidget *widget, gpointer user_data);

static void register_panel_make_archive(GtkWidget *widget, gpointer user_data);

static void reconcile_selected_foreach_func (GtkTreeModel	*model, GtkTreePath	 *path, GtkTreeIter	 *iter, gpointer			 userdata);
Transaction *get_active_transaction(GtkTreeView *treeview);
static void register_panel_selection(GtkTreeSelection *treeselection, gpointer user_data);
static void register_panel_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);



/* account action functions -------------------- */


static void register_panel_action_editfilter(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_FILTER));
}

static void register_panel_action_add(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_ADD));
}

static void register_panel_action_inherit(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_INHERIT));
}

static void register_panel_action_edit(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_EDIT));
}

static void register_panel_action_reconcile(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_RECONCILE));
}

static void register_panel_action_remove(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_action(data->window, GINT_TO_POINTER(ACTION_ACCOUNT_REMOVE));
}



static void register_panel_action_createtemplate(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_make_archive(data->window, NULL);
}



static void register_panel_action_exportcsv(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	register_panel_export_csv(data->window, NULL);
}


static void register_panel_action_exportqif(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;
gchar *filename;

	DB( g_print("(qif) test qif export\n\n") );

	if( ui_file_chooser_qif(GTK_WINDOW(data->window), &filename) == TRUE )
	{
		hb_export_qif_account_single(filename, data->acc);
		g_free( filename );
	}
}


static void register_panel_action_assign(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;
gint count;
gboolean usermode = TRUE;


	count = transaction_auto_assign(GLOBALS->ope_list, data->accnum);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_ope));
	GLOBALS->changes_count += count;

	//inform the user
	if(usermode == TRUE)
	{
	gchar *txt;

		if(count == 0)
			txt = _("No transaction changed");
		else
			txt = _("transaction auto assigned: %d");

		ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_INFO,
			_("Auto assigment result"),
			txt,
			count);
	}

}


static void register_panel_action_close(GtkAction *action, gpointer user_data)
{
struct register_panel_data *data = user_data;

	DB( g_print("action close\n") );

	DB( g_print("window %p\n", data->window) );

	gtk_widget_destroy (GTK_WIDGET (data->window));

	//g_signal_emit_by_name(data->window, "delete-event", NULL, &result);

}


/* these 5 functions are independant from account window */

/* account functions -------------------- */

static void register_panel_export_csv(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
gchar *filename = NULL;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
GIOChannel *io;

	DB( g_print("\n[account] export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
	{

		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			//title line
			g_io_channel_write_chars(io, "date;paymode;info;payee;wording;amount;category;tags\n", -1, NULL, NULL);


			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));

			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Transaction *ope;
			gchar *outstr;
			GDate *date;
			gchar datebuf[256];
			gchar *info, *payeename, *categoryname;
			Payee *payee;
			Category *category;
			gchar *tags;
			char amountbuf[G_ASCII_DTOSTR_BUF_SIZE];

				gtk_tree_model_get (model, &iter,
						LST_DSPOPE_DATAS, &ope,
						-1);

			//date
				date = g_date_new_julian (ope->date);
				if( PREFS->dtex_datefmt == PRF_DATEFMT_MDY )
				{
					g_sprintf(datebuf, "%02d/%02d/%04d",
						g_date_get_month(date),
						g_date_get_day(date),
						g_date_get_year(date)
						);
				}
				else
				{
					g_sprintf(datebuf, "%02d/%02d/%04d",
						g_date_get_day(date),
						g_date_get_month(date),
						g_date_get_year(date)
						);
				}	
					
				g_date_free(date);

				info = ope->info;
				if(info == NULL) info = "";
				payee = da_pay_get(ope->kpay);
				payeename = (payee->name == NULL) ? "" : payee->name;
				category = da_cat_get(ope->kcat);
				categoryname = (category->name == NULL) ? "" : da_cat_get_fullname(category);
				tags = transaction_tags_tostring(ope);

				//#793719
				//g_ascii_dtostr (amountbuf, sizeof (amountbuf), ope->amount);
				g_ascii_formatd (amountbuf, sizeof (amountbuf), "%.2f", ope->amount);



				DB( g_print("amount = %f '%s'\n", ope->amount, amountbuf) );


				outstr = g_strdup_printf("%s;%d;%s;%s;%s;%s;%s;%s\n",
						datebuf,
						ope->paymode,
						info,
						payeename,
						ope->wording,
						amountbuf,
						categoryname,
						tags != NULL ? tags : ""
						);

				DB( g_print("%s", outstr) );

				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				g_free(outstr);
				g_free(tags);


				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

		g_free( filename );

	}

}



/*
** make an archive with the active transaction
*/
static void register_panel_make_archive(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;

	DB( g_print("\n[account] make archive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

GtkWidget *p_dialog = NULL;
GtkTreeModel *model;
GList *selection, *list;
gint result, count;

	count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)));

	if( count > 0 )
	{
			p_dialog = gtk_message_dialog_new
			(
				NULL,
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
			GTK_BUTTONS_YES_NO,
			_("Do you want to create a template with\neach of the selected transaction ?")
			);

	/*
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
			_("%d archives will be created"),
			GLOBALS->changes_count
			);
	*/

		result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
		gtk_widget_destroy( p_dialog );


		if(result == GTK_RESPONSE_YES)
		{

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
			selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)), &model);

			list = g_list_first(selection);
			while(list != NULL)
			{
			Archive *item;
			Transaction *ope;
			GtkTreeIter iter;

				gtk_tree_model_get_iter(model, &iter, list->data);
				gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);

				DB( g_print(" create archive %s %.2f\n", ope->wording, ope->amount) );

				item = da_archive_malloc();

				//fill it
				item->amount		= ope->amount;
				item->kacc		= ope->kacc;
				item->kxferacc	= ope->kxferacc;
				item->paymode		= ope->paymode;
				item->flags			= ope->flags	& (OF_INCOME);
				item->kpay			= ope->kpay;
				item->kcat		= ope->kcat;
				if(ope->wording != NULL)
					item->wording 		= g_strdup(ope->wording);
				else
					item->wording 		= g_strdup(_("(new archive)"));

				GLOBALS->arc_list = g_list_append(GLOBALS->arc_list, item);
				GLOBALS->changes_count++;

				list = g_list_next(list);
			}

			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}
	}
}


static void register_panel_cb_filter_daterange(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
gint range;

	DB( g_print("\n[account] filter_daterange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range);
		register_panel_collect_filtered_txn(data->LV_ope);
		register_panel_listview_populate(data->LV_ope);
	}
	else
	{
		if(ui_flt_manage_dialog_new(data->filter, FALSE) != GTK_RESPONSE_REJECT)
		{
			register_panel_collect_filtered_txn(data->LV_ope);
			register_panel_listview_populate(data->LV_ope);
			register_panel_update(data->LV_ope, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));
		}
	}
	
	
}

static void register_panel_cb_filter_type(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
gint type;

	DB( g_print("\n[account] filter_type\n") );
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));

	filter_preset_type_set(data->filter, type);

	register_panel_collect_filtered_txn(data->LV_ope);
	register_panel_listview_populate(data->LV_ope);
}


static void register_panel_cb_filter_status(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
gint status;

	DB( g_print("\n[account] filter_status\n") );
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	status = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_status));

	filter_preset_status_set(data->filter, status);

	register_panel_collect_filtered_txn(data->LV_ope);
	register_panel_listview_populate(data->LV_ope);
}


static void register_panel_cb_filter_reset(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;

	DB( g_print("\n[account] filter_reset\n") );
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	filter_default_all_set(data->filter);
	filter_preset_daterange_set (data->filter, PREFS->date_range_txn);
	if(PREFS->hidereconciled)
		filter_preset_status_set (data->filter, 1);

	register_panel_collect_filtered_txn(data->LV_ope);
	register_panel_listview_populate(data->LV_ope);
	
	g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
	g_signal_handler_block(data->CY_type, data->handler_id[HID_TYPE]);
	g_signal_handler_block(data->CY_status, data->handler_id[HID_STATUS]);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), data->filter->range);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_type), data->filter->type);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_status), data->filter->status);

	g_signal_handler_unblock(data->CY_status, data->handler_id[HID_STATUS]);
	g_signal_handler_unblock(data->CY_type, data->handler_id[HID_TYPE]);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);
	
}


static void register_panel_balance_refresh(GtkWidget *view)
{
struct register_panel_data *data;
GList *list;
gdouble balance;
GtkTreeModel *model;
guint32 ldate = 0;
gushort lpos = 1;
	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(view, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[account] balance refresh\n") );

	balance = data->acc->initial;

	//#1270687: sort if date changed
	if(data->do_sort)
	{
		DB( g_print(" - complete txn sort\n") );
		GLOBALS->ope_list = da_transaction_sort(GLOBALS->ope_list);
		data->do_sort = FALSE;
	}

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Transaction *ope;

		ope = list->data;
		if(ope->kacc == data->accnum)
		{	
			//#1267344
			if(!(ope->flags & OF_REMIND))
				balance += ope->amount;

			ope->balance = balance;
		}

		if(ope->date == ldate)
		{
			ope->pos = ++lpos;	
		}
		else
		{
			ope->pos = lpos = 1;
		}
		ldate = ope->date;

		list = g_list_next(list);
	}
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
	list_transaction_sort_force(GTK_TREE_SORTABLE(model), NULL);
	
}


static void register_panel_collect_filtered_txn(GtkWidget *view)
{
struct register_panel_data *data;
guint i;
GList *list;

	DB( g_print("\n[register_panel] collect_filtered_txn\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(view, GTK_TYPE_WINDOW)), "inst_data");

	if(data->gpatxn != NULL)
		g_ptr_array_free (data->gpatxn, TRUE);

#if MYDEBUG == 1
	guint nbtxn = g_list_length (GLOBALS->ope_list);
	g_print(" - nb txn %d\n", nbtxn);
#endif
	
	data->gpatxn = g_ptr_array_sized_new(64);

	//data->hidden = 0;

	list = g_list_first(GLOBALS->ope_list); i=0;
	while (list != NULL)
	{
	Transaction *ope = list->data;

		if(ope->kacc == data->accnum)
		{
			if(filter_test(data->filter, ope) == 1)
			{
				g_ptr_array_add(data->gpatxn, (gpointer)ope);
			}
			/*else
			{
				data->hidden++;
			}*/
		}

#if MYDEBUG == 1
		
		if( !(i % 1000) ) { g_print(" - progress %d/%d\n", i, nbtxn); }

#endif
		i++;
		list = g_list_next(list);
	}

	g_signal_handler_block(data->ST_search, data->handler_id[HID_SEARCH]);
	gtk_entry_set_text (GTK_ENTRY(data->ST_search), "");
	g_signal_handler_unblock(data->ST_search, data->handler_id[HID_SEARCH]);

	
}


static void register_panel_listview_populate(GtkWidget *widget)
{
struct register_panel_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean hastext;
gchar *needle;
gint sort_column_id;
GtkSortType order;
guint i, qs_flag;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[register_panel] listview_populate\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	// ref model to keep it
	g_object_ref(model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), NULL);

	// perf: if you leave the sort, insert is damned slow
	gtk_tree_sortable_get_sort_column_id (GTK_TREE_SORTABLE(GTK_LIST_STORE(model)), &sort_column_id, &order);
	
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(GTK_LIST_STORE(model)), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, PREFS->lst_ope_sort_order);

	hastext = gtk_entry_get_text_length (GTK_ENTRY(data->ST_search)) >= 2;
	needle = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_search));

	//build the mask flag for quick search
	qs_flag = 0;
	if(hastext)
	{
		qs_flag = list_transaction_get_quicksearch_column_mask(GTK_TREE_VIEW(data->LV_ope));
	}
	
	data->total = 0;
	data->totalsum = 0.0;

	for(i=0;i<data->gpatxn->len;i++)
	{
	Transaction *txn = g_ptr_array_index(data->gpatxn, i);
	gboolean insert = TRUE;
		
		if(hastext)
		{
			insert = filter_txn_search_match(needle, txn, qs_flag);
		}

		if(insert)
		{
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);

	 		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, txn,
				-1);

			data->totalsum += txn->amount;
			data->total++;
		}
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), model); /* Re-attach model to view */
	g_object_unref(model);

	// push back the sort id
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(GTK_LIST_STORE(model)), sort_column_id, order);

	/* update info range text */
	{
	gchar *daterange;
		
		daterange = filter_daterange_text_get(data->filter);
		gtk_widget_set_tooltip_markup(GTK_WIDGET(data->CY_range), daterange);
		g_free(daterange);
	}
	
	register_panel_update(data->LV_ope, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));

}


static void reconcile_selected_foreach_func (GtkTreeModel	*model, GtkTreePath	 *path, GtkTreeIter	 *iter, gpointer			 userdata)
{
Transaction *trn;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &trn, -1);

	account_balances_sub(trn);

	trn->flags ^= OF_VALID;
	//#1308745 remind flags is exclusive with reconciled
	if(trn->flags & OF_VALID)
		trn->flags &= ~(OF_REMIND);

	trn->flags |= OF_CHANGED;

	account_balances_add(trn);

	/* #492755 let the chld transfer unchnaged */

}


Transaction *get_active_transaction(GtkTreeView *treeview)
{
GtkTreeModel *model;
GList *list;
Transaction *ope;

	ope = NULL;

	model = gtk_tree_view_get_model(treeview);
	list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

	if(list != NULL)
	{
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);

	return ope;
}


static void remove_active_transaction(GtkTreeView *treeview)
{
GtkTreeModel *model;
GList *list;

	model = gtk_tree_view_get_model(treeview);
	list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

	if(list != NULL)
	{
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);


}



static void register_panel_action(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
gint action = GPOINTER_TO_INT(user_data);
gboolean result;

	DB( g_print("\n[account] action\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	DB( g_print(" - action=%d\n", action) );

	switch(action)
	{
		//add
		case ACTION_ACCOUNT_ADD:
		//inherit
		case ACTION_ACCOUNT_INHERIT:
		{
		GtkWidget *dialog;
		Transaction *src_txn;
		gint type = 0;

			homebank_app_date_get_julian();
			
			if(action == ACTION_ACCOUNT_ADD)
			{
				DB( g_print("(transaction) add multiple\n") );
				src_txn = da_transaction_malloc();
				src_txn->date = GLOBALS->today;
				src_txn->kacc = data->accnum;
				type = TRANSACTION_EDIT_ADD;
			}
			else
			{
				DB( g_print("(transaction) inherit multiple\n") );
				src_txn = da_transaction_clone(get_active_transaction(GTK_TREE_VIEW(data->LV_ope)));
				type = TRANSACTION_EDIT_INHERIT;
			}

			dialog = create_deftransaction_window(GTK_WINDOW(data->window), type);
			result = GTK_RESPONSE_ADD;
			while(result == GTK_RESPONSE_ADD)
			{
				/* clone source transaction */
				data->cur_ope = da_transaction_clone (src_txn);

				if( PREFS->heritdate == FALSE ) //fix: 318733 / 1335285
					data->cur_ope->date = GLOBALS->today;

				deftransaction_set_transaction(dialog, data->cur_ope);

				result = gtk_dialog_run (GTK_DIALOG (dialog));
				if(result == GTK_RESPONSE_ADD || result == GTK_RESPONSE_ACCEPT)
				{
					deftransaction_get(dialog, NULL);
					transaction_add(data->cur_ope, data->LV_ope, data->accnum);
					register_panel_update(widget, GINT_TO_POINTER(UF_BALANCE));
					data->acc->flags |= AF_ADDED;
					GLOBALS->changes_count++;
					//store last date
					src_txn->date = data->cur_ope->date;
				}

				da_transaction_free (data->cur_ope);
			}
			deftransaction_dispose(dialog, NULL);
			da_transaction_free (src_txn);

			gtk_widget_destroy (dialog);
		}
		break;

		case ACTION_ACCOUNT_EDIT:
			{
		Transaction *old_txn;
		GtkWidget *dialog;

			old_txn = get_active_transaction(GTK_TREE_VIEW(data->LV_ope));
			if(old_txn)
			{
				dialog = create_deftransaction_window(GTK_WINDOW(data->window), TRANSACTION_EDIT_MODIFY);

				data->cur_ope = da_transaction_clone (old_txn); // to keep old datas, just in case
				deftransaction_set_transaction(dialog, data->cur_ope);
				
				result = gtk_dialog_run (GTK_DIALOG (dialog));
				if(result == GTK_RESPONSE_ACCEPT)
				{
					deftransaction_get(dialog, NULL);

					account_balances_sub(old_txn);
					account_balances_add(data->cur_ope);

					// different accoutn : remove from the display
					if( data->cur_ope->kacc != data->accnum )
					{
						remove_active_transaction(GTK_TREE_VIEW(data->LV_ope));
					}

					if( data->cur_ope->paymode == PAYMODE_INTXFER )
					{
						//nota: if kxfer is 0, the user may have just changed the paymode to xfer
						DB( g_print(" - kxfer = %d\n", data->cur_ope->kxfer) );

						if(data->cur_ope->kxfer > 0)	//1) search a strong linked child
						{
						Transaction *ltxn;

							DB( g_print(" - old_txn: kacc=%d kxferacc=%d\n", old_txn->kacc, old_txn->kxferacc) );
							
							ltxn = transaction_strong_get_child_transfer(old_txn);
							if(ltxn != NULL) //should never be the case
							{
								DB( g_print(" - strong link found, do sync\n") );
								transaction_xfer_sync_child(data->cur_ope, ltxn);
							}
							else
							{
								DB( g_print(" - no, somethin' went wrong here...\n") );
							}
						}
						else
						{
							//2) any standard transaction that match ?
							transaction_xfer_search_or_add_child(data->cur_ope, data->LV_ope);
						}
					}

					//#1250061 : manage ability to break an internal xfer
					if(old_txn->paymode == PAYMODE_INTXFER && data->cur_ope->paymode != PAYMODE_INTXFER)
					{
					GtkWidget *p_dialog;
						
						DB( g_print(" - should break internal xfer\n") );

						p_dialog = gtk_message_dialog_new
						(
							NULL,
							GTK_DIALOG_MODAL,
							GTK_MESSAGE_WARNING,
							GTK_BUTTONS_YES_NO,
							_("Do you want to break the internal transfer ?\n\n"
							  "Proceeding will delete the target transaction.")
						);

						result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
						gtk_widget_destroy( p_dialog );

						if(result == GTK_RESPONSE_YES)
						{
							transaction_xfer_delete_child(data->cur_ope);
						}
						else	//force paymode to internal xfer
						{
							data->cur_ope->paymode = PAYMODE_INTXFER;
						}
					}
					
					//#1270687: sort if date changed
					if(old_txn->date != data->cur_ope->date)
						data->do_sort = TRUE;
					
					da_transaction_copy(data->cur_ope, old_txn);

					register_panel_update(widget, GINT_TO_POINTER(UF_BALANCE));

					data->acc->flags |= AF_CHANGED;
					GLOBALS->changes_count++;

				}

				da_transaction_free (data->cur_ope);

			
				deftransaction_dispose(dialog, NULL);
				gtk_widget_destroy (dialog);
			}

		}
		break;

		case ACTION_ACCOUNT_REMOVE:
		{
		GtkWidget *p_dialog = NULL;
		GtkTreeModel *model;
		GList *selection, *list;
		gint result;
		//gint count;

			DB( g_print(" - remove\n") );

			//count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)));

				//todo: replace with a call to ui_dialog_msg_question ?

				p_dialog = gtk_message_dialog_new
				(
					NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_WARNING,
				GTK_BUTTONS_YES_NO,
				_("Do you want to delete\neach of the selected transaction ?")
				);

			/*
			gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
				_("%d transactions will be definitively lost.\n"),
				GLOBALS->changes_count
				);
			*/


			result = gtk_dialog_run( GTK_DIALOG( p_dialog ) );
			gtk_widget_destroy( p_dialog );


			if(result == GTK_RESPONSE_YES)
			{

				model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));
				selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope)), &model);

				g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
				gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), NULL); /* Detach model from view */


				DB( g_print(" remove %d line\n", g_list_length(selection)) );


				list = g_list_last(selection);
				while(list != NULL)
				{
				Transaction *entry;
				GtkTreeIter iter;

					gtk_tree_model_get_iter(model, &iter, list->data);
					gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &entry, -1);

					DB( g_print(" delete %s %.2f\n", entry->wording, entry->amount) );

					account_balances_sub(entry);

					/* v3.4: also remove child transfer */
					if( entry->paymode == PAYMODE_INTXFER )
					{
						transaction_xfer_delete_child( entry );
					}

					gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
					GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, entry);
					da_transaction_free(entry);

					GLOBALS->changes_count++;


					list = g_list_previous(list);
				}

				g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
				g_list_free(selection);

					gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_ope), model); /* Re-attach model to view */
					g_object_unref(model);

				register_panel_update(widget, GINT_TO_POINTER(UF_BALANCE));

				data->acc->flags |= AF_CHANGED;

			}
		}
		break;

		//reconcile
		case ACTION_ACCOUNT_RECONCILE:
		{
			GtkTreeSelection *selection;

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
			gtk_tree_selection_selected_foreach(selection, (GtkTreeSelectionForeachFunc)reconcile_selected_foreach_func, data);

			DB( g_print(" - reconcile\n") );

			gtk_widget_queue_draw (data->LV_ope);
			//gtk_widget_queue_resize (data->LV_acc);


			register_panel_update(widget, GINT_TO_POINTER(UF_BALANCE));

			data->acc->flags |= AF_CHANGED;
			GLOBALS->changes_count++;

		}

			break;

		case ACTION_ACCOUNT_FILTER:
		{

			if(ui_flt_manage_dialog_new(data->filter, FALSE) != GTK_RESPONSE_REJECT)
			{
				register_panel_collect_filtered_txn(data->LV_ope);
				register_panel_listview_populate(data->LV_ope);
				register_panel_update(data->LV_ope, GINT_TO_POINTER(UF_SENSITIVE+UF_BALANCE));

				g_signal_handler_block(data->CY_range, data->handler_id[HID_RANGE]);
				gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), FLT_RANGE_OTHER);
				g_signal_handler_unblock(data->CY_range, data->handler_id[HID_RANGE]);
			}

		}
		break;

	}

}



static void register_panel_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;

	DB( g_print("\n[account] toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	register_panel_update(data->LV_ope, GINT_TO_POINTER(UF_BALANCE));
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_ope));
}


static void register_panel_selection(GtkTreeSelection *treeselection, gpointer user_data)
{

	DB( g_print("\n[account] selection changed cb\n") );


	register_panel_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(UF_SENSITIVE));

}


static void register_panel_update(GtkWidget *widget, gpointer user_data)
{
struct register_panel_data *data;
GtkTreeSelection *selection;
gint flags = GPOINTER_TO_INT(user_data);
gint count = 0;

	DB( g_print("\n[account] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//data = INST_DATA(widget);

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


	/* set window title */
	if(flags & UF_TITLE)
	{
		DB( printf(" - UF_TITLE\n") );

	}

	/* update disabled things */
	if(flags & UF_SENSITIVE)
	{
	gboolean	sensitive;

		DB( printf(" - UF_SENSITIVE\n") );

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
		count = gtk_tree_selection_count_selected_rows(selection);
		DB( printf(" - count = %d\n", count) );


	/*
		if (active = gtk_tree_selection_get_selected(selection, &model, &iter))
		{
		gint *indices;

			path = gtk_tree_model_get_path(model, &iter);
			indices = gtk_tree_path_get_indices(path);

			data->accnum = indices[0];

			DB( printf(" active is %d, sel=%d\n", indices[0], active) );
		}
		*/

		// multiple: disable inherit, edit
		sensitive = (count != 1 ) ? FALSE : TRUE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/TxnBar/Inherit"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/TxnBar/Edit"), sensitive);
		
		sensitive = (count > 0 ) ? TRUE : FALSE;
		// no selection: disable reconcile, remove
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/TransactionMenu/Reconcile"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/TransactionMenu/Remove"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/TransactionMenu/Template"), sensitive);

		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/TxnBar/Remove"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/TxnBar/Reconcile"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/TxnBar/Template"), sensitive);


		// multiple: disable inherit, edit
		sensitive = (count != 1 ) ? FALSE : TRUE;
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/TransactionMenu/Inherit"), sensitive);
		gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/MenuBar/TransactionMenu/Edit"), sensitive);

	}

	/* update toolbar & list */
	if(flags & UF_VISUAL)
	{
		DB( printf(" - UF_VISUAL\n") );

		if(PREFS->toolbar_style == 0)
			gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
		else
			gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

		//minor ?
		if( PREFS->euro_active )
		{
			gtk_widget_show(data->CM_minor);
		}
		else
		{
			gtk_widget_hide(data->CM_minor);
		}
	}

	/* update balances */
	if(flags & UF_BALANCE)
	{
		DB( printf(" - UF_BALANCE\n") );

		register_panel_balance_refresh(widget);
		
		/*
		hb_label_set_colvaluecurr(GTK_LABEL(data->TX_balance[0]), data->acc->bal_bank, data->acc->kcur);
		hb_label_set_colvaluecurr(GTK_LABEL(data->TX_balance[1]), data->acc->bal_today, data->acc->kcur);
		hb_label_set_colvaluecurr(GTK_LABEL(data->TX_balance[2]), data->acc->bal_future, data->acc->kcur);
		*/
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[0]), data->acc->bal_bank, GLOBALS->minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[1]), data->acc->bal_today, GLOBALS->minor);
		hb_label_set_colvalue(GTK_LABEL(data->TX_balance[2]), data->acc->bal_future, GLOBALS->minor);

		ui_mainwindow_populate_accounts(GLOBALS->mainwindow, NULL);
	}

	/* update fltinfo */
	DB( printf(" - statusbar\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_ope));
	count = gtk_tree_selection_count_selected_rows(selection);
	DB( printf(" - nb selected = %d\n", count) );

	/* if more than one ope selected, we make a sum to display to the user */
	gdouble opeexp = 0.0;
	gdouble opeinc = 0.0;
	gchar buf1[64];
	gchar buf2[64];
	gchar buf3[64];

	if( count >= 1 )
	{
	GList *list, *tmplist;
	GtkTreeModel *model;
	GtkTreeIter iter;

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope));

		list = gtk_tree_selection_get_selected_rows(selection, &model);

		tmplist = g_list_first(list);
		while (tmplist != NULL)
		{
		Transaction *item;

			gtk_tree_model_get_iter(model, &iter, tmplist->data);
			gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &item, -1);

			if( item->flags & OF_INCOME )
				opeinc += item->amount;
			else
				opeexp += item->amount;

			DB( printf(" - %s, %.2f\n", item->wording, item->amount ) );

			tmplist = g_list_next(tmplist);
		}
		g_list_free(list);

		DB( printf(" %f - %f = %f\n", opeinc, opeexp, opeinc + opeexp) );

		/*
		hb_strfmon(buf1, 64-1, opeinc, data->acc->kcur);
		hb_strfmon(buf2, 64-1, -opeexp, data->acc->kcur);
		hb_strfmon(buf3, 64-1, opeinc + opeexp, data->acc->kcur);
		*/
		mystrfmon(buf1, 64-1, opeinc, GLOBALS->minor);
		mystrfmon(buf2, 64-1, -opeexp, GLOBALS->minor);
		mystrfmon(buf3, 64-1, opeinc + opeexp, GLOBALS->minor);
	}

	gchar *msg;

	if( count <= 0 )
	{
		//msg = g_strdup_printf (_("transaction selected: %d, hidden: %d"), count, data->hidden);
		mystrfmon(buf3, 64-1, data->totalsum, GLOBALS->minor);
		msg = g_strdup_printf(_("%d items (%s)"), data->total, buf3);
	}
	else
		//TRANSLATORS: detail of the 3 %s which are some amount of selected transaction, 1=total 2=income, 3=expense
		//msg = g_strdup_printf (_("transaction selected: %d, hidden: %d / %s ( %s - %s)"), count, data->hidden, buf3, buf1, buf2);
		msg = g_strdup_printf(_("%d items (%d selected %s)"), data->total, count, buf3);

	gtk_label_set_markup(GTK_LABEL(data->TX_selection), msg);
	g_free (msg);

}


void register_panel_onRowActivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
struct register_panel_data *data;
GtkTreeModel *model;
GtkTreeIter iter;
gint col_id, count;
GList *selection, *list;
Transaction *ope;
gchar *tagstr, *txt;
gboolean refreshbalance;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	col_id = gtk_tree_view_column_get_sort_column_id (col);

	count = gtk_tree_selection_count_selected_rows(gtk_tree_view_get_selection(treeview));

	model = gtk_tree_view_get_model(treeview);

	//get transaction double clicked to initiate the widget
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);


		DB( g_print ("%d rows been double-clicked on column=%d! ope=%s\n", count, col_id, ope->wording) );

	if( count == 1)
	{
		register_panel_action(GTK_WIDGET(treeview), GINT_TO_POINTER(ACTION_ACCOUNT_EDIT));
	}
	else
	if(col_id >= LST_DSPOPE_DATE && col_id != LST_DSPOPE_BALANCE)
	{
	GtkWidget *parentwindow, *dialog, *mainbox, *widget1, *widget2, *content;

		parentwindow = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);

		dialog = gtk_dialog_new_with_buttons (NULL,
							GTK_WINDOW (parentwindow),
							0,
							GTK_STOCK_CANCEL,
							GTK_RESPONSE_REJECT,
							GTK_STOCK_OK,
							GTK_RESPONSE_ACCEPT,
							NULL);

		gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_MOUSE);
		gint w;
		w = gtk_tree_view_column_get_width(GTK_TREE_VIEW_COLUMN(col));
		gtk_window_set_default_size (GTK_WINDOW(dialog), w + 40, 0);

		content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

		mainbox = gtk_hbox_new (FALSE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainbox), HB_BOX_SPACING);
		gtk_box_pack_start (GTK_BOX (content), mainbox, FALSE, FALSE, 0);

		widget1 = widget2 = NULL;

		switch( col_id )
		{
			case LST_DSPOPE_DATE:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify date..."));
				widget1 = gtk_dateentry_new();
				gtk_dateentry_set_date(GTK_DATE_ENTRY(widget1), (guint)ope->date);
				break;
			case LST_DSPOPE_INFO:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify info..."));
				widget1 = make_paymode(NULL);
				widget2 = make_string(NULL);
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget1), ope->paymode);
				gtk_entry_set_text(GTK_ENTRY(widget2), ope->info);
				break;
			case LST_DSPOPE_PAYEE:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify payee..."));
				widget1 = ui_pay_comboboxentry_new(NULL);
				ui_pay_comboboxentry_populate(GTK_COMBO_BOX(widget1), GLOBALS->h_pay);
				ui_pay_comboboxentry_set_active(GTK_COMBO_BOX(widget1), ope->kpay);
				break;
			case LST_DSPOPE_WORDING:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify description..."));
				widget1 = make_string(NULL);
				gtk_entry_set_text(GTK_ENTRY(widget1), ope->wording);
				break;
			case LST_DSPOPE_EXPENSE:
			case LST_DSPOPE_INCOME:
			case LST_DSPOPE_AMOUNT:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify amount..."));
				widget1 = make_amount(NULL);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget1), ope->amount);
				break;
			case LST_DSPOPE_CATEGORY:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify category..."));
				widget1 = ui_cat_comboboxentry_new(FALSE);
				ui_cat_comboboxentry_populate(GTK_COMBO_BOX(widget1), GLOBALS->h_cat);
				ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(widget1), ope->kcat);
				break;
			case LST_DSPOPE_TAGS:
				gtk_window_set_title (GTK_WINDOW (dialog), _("Modify tags..."));
				widget1 = make_string(NULL);

				tagstr = transaction_tags_tostring(ope);

				txt = (tagstr != NULL) ? tagstr : "";
				gtk_entry_set_text(GTK_ENTRY(widget1), txt);
				g_free(tagstr);

				break;
		}

		if(widget1 != NULL) gtk_box_pack_start (GTK_BOX (mainbox), widget1, TRUE, TRUE, 0);
		if(widget2 != NULL) gtk_box_pack_start (GTK_BOX (mainbox), widget2, TRUE, TRUE, 0);

		gtk_widget_show_all(mainbox);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_ACCEPT)
		{
			selection = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

			refreshbalance = FALSE;
			
			list = g_list_first(selection);
			while(list != NULL)
			{
			GtkTreeIter iter;
			const gchar *txt;

				gtk_tree_model_get_iter(model, &iter, list->data);
				gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);

				DB( g_print(" modifying %s %.2f\n", ope->wording, ope->amount) );

				switch( col_id )
				{
					case LST_DSPOPE_DATE:
						ope->date = gtk_dateentry_get_date(GTK_DATE_ENTRY(widget1));
						data->do_sort = TRUE;
						refreshbalance = TRUE;
						break;
					case LST_DSPOPE_INFO:
						ope->paymode = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
						txt = gtk_entry_get_text(GTK_ENTRY(widget2));
						if (txt && *txt)
						{
							g_free(ope->info);
							ope->info = g_strdup(txt);
						}
						break;
					case LST_DSPOPE_PAYEE:
						ope->kpay = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(widget1));
						DB( g_print(" -> payee: '%d'\n", ope->kpay) );
						break;
					case LST_DSPOPE_WORDING:
						txt = gtk_entry_get_text(GTK_ENTRY(widget1));
						if (txt && *txt)
						{
							g_free(ope->wording);
							ope->wording = g_strdup(txt);
						}
						break;
					case LST_DSPOPE_EXPENSE:
					case LST_DSPOPE_INCOME:
					case LST_DSPOPE_AMOUNT:
						ope->flags &= ~(OF_INCOME);	//remove flag
						ope->amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget1));
						if(ope->amount > 0) ope->flags |= OF_INCOME;
						refreshbalance = TRUE;
						break;
					case LST_DSPOPE_CATEGORY:
						if(!(ope->flags & OF_SPLIT))
						{
							ope->kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(widget1));
							//bad .... ope->category = gtk_combo_box_get_active(GTK_COMBO_BOX(widget1));
							DB( g_print(" -> category: '%d'\n", ope->kcat) );
						}
						break;
					case LST_DSPOPE_TAGS:
						txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(widget1));
						if (txt && *txt)
						{
							DB( g_print(" -> tags: '%s'\n", txt) );

							transaction_tags_parse(ope, txt);
						}

						break;

				}

				ope->flags |= OF_CHANGED;
				GLOBALS->changes_count++;

				list = g_list_next(list);
			}

			if(refreshbalance)
				register_panel_update(GTK_WIDGET(treeview), GINT_TO_POINTER(UF_BALANCE));


			g_list_foreach(selection, (GFunc)gtk_tree_path_free, NULL);
			g_list_free(selection);
		}

		// cleanup and destroy
		gtk_widget_destroy (dialog);

	}
}




/*
static gint listview_context_cb (GtkWidget *widget, GdkEventButton *event, GtkWidget *menu)
{

	if (event->button == 3)
	{


		if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (treeview),
			(gint) event->x, (gint) event->y, &path, NULL, NULL, NULL))
		{
			gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);
			gtk_tree_path_free (path);
		}




				gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL,
												event->button, event->time);

				// On indique à l'appelant que l'on a géré cet événement.

				return TRUE;
		}

		// On indique à l'appelant que l'on n'a pas géré cet événement.

		return FALSE;
}
*/


/*
** populate the account window
*/
void register_panel_window_init(GtkWidget *widget, gpointer user_data)
{

	DB( g_print("\n[account] init window\n") );

	DB( g_print(" - sort transactions\n") );
	GLOBALS->ope_list = da_transaction_sort(GLOBALS->ope_list);

	//DB( g_print(" mindate=%d, maxdate=%d %x\n", data->filter->mindate,data->filter->maxdate) );

	DB( g_print(" - call update visual\n") );
	register_panel_update(widget, GINT_TO_POINTER(UF_VISUAL));

	DB( g_print(" - set range or populate+update sensitive+balance\n") );
	
	register_panel_cb_filter_reset(widget, user_data);

}

/*
**
*/
static gboolean 
register_panel_getgeometry(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
//struct register_panel_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("\n[account] get geometry\n") );

	//store position and size
	wg = &PREFS->acc_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);
	GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(widget));
	GdkWindowState state = gdk_window_get_state(gdk_window);
	wg->s = (state & GDK_WINDOW_STATE_MAXIMIZED) ? 1 : 0;
	
	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d s=%d, state=%d\n", wg->l, wg->t, wg->w, wg->h, wg->s, state & GDK_WINDOW_STATE_MAXIMIZED) );

	return FALSE;
}

/*
**
*/
static gboolean register_panel_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
//struct register_panel_data *data = user_data;


	DB( g_print("\n[account] delete-event\n") );


	return FALSE;
}

/* Another callback */
static gboolean register_panel_destroy( GtkWidget *widget,
										 gpointer	 user_data )
{
struct register_panel_data *data;

	data = g_object_get_data(G_OBJECT(widget), "inst_data");


	DB( g_print ("\n[account] destroy event occurred\n") );



	//enable define windows
	GLOBALS->define_off--;

	/* unset transaction edit mutex */
	if(data->acc)
		data->acc->window = NULL;

	/* free title and filter */
	DB( g_print(" user_data=%p to be free\n", user_data) );
	g_free(data->wintitle);

	if(data->gpatxn != NULL)
		g_ptr_array_free (data->gpatxn, TRUE);

	da_filter_free(data->filter);

	g_free(data);


	//our global list has changed, so update the treeview
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_REFRESHALL));

	return FALSE;
}


static void 
quick_search_activate_cb (GtkEntry  *entry, gpointer  user_data)
{
struct register_panel_data *data = user_data;
	
	register_panel_listview_populate (data->LV_ope);
}


static gint quick_search_text_changed_timeout (gpointer user_data)
{
struct register_panel_data *data = user_data;
	
	DB( g_print("quick search timed out !\n") );

	register_panel_listview_populate (data->window);

	data->timer_tag = 0;

	return FALSE;
}

static void
quick_search_text_changed_cb (GtkEntry   *entry,
                 GParamSpec *pspec,
                 gpointer user_data)
{
struct register_panel_data *data = user_data;

	gboolean has_text;

  has_text = gtk_entry_get_text_length (entry) > 0;
  gtk_entry_set_icon_sensitive (entry,
                                GTK_ENTRY_ICON_SECONDARY,
                                has_text);

	if(data->timer_tag == 0 )
		data->timer_tag = g_timeout_add( DEFAULT_DELAY, quick_search_text_changed_timeout, (gpointer)user_data);


	
}

static void
quick_search_icon_press_cb (GtkEntry       *entry,
               gint            position,
               GdkEventButton *event,
               gpointer        data)
{
  if (position == GTK_ENTRY_ICON_SECONDARY)
    gtk_entry_set_text (entry, "");
}


static GtkActionEntry entries[] = {

	/* name, stock id, label */
	{ "AccountMenu"	, NULL, N_("_Account"), NULL, NULL, NULL },
	{ "TransactionMenu", NULL, N_("Transacti_on"), NULL, NULL, NULL },
	{ "ActionsMenu"	, NULL, N_("_Actions"), NULL, NULL, NULL },
	{ "ToolsMenu"		, NULL, N_("_Tools"), NULL, NULL, NULL },

	{ "Close"				, GTK_STOCK_CLOSE	, N_("_Close")				, NULL, N_("Close the current account"),		G_CALLBACK (register_panel_action_close) },

	/* name, stock id, label, accelerator, tooltip */
	{ "Filter"			 , "hb-filter"			, N_("_Filter..."), NULL,		N_("Open the list filter"), G_CALLBACK (register_panel_action_editfilter) },

	{ "Add"					, HB_STOCK_OPE_ADD		, N_("_Add..."), NULL,		N_("Add a new transaction"), G_CALLBACK (register_panel_action_add) },
	{ "Inherit"			, HB_STOCK_OPE_HERIT	, N_("_Inherit..."), NULL, N_("Inherit from the active transaction"), G_CALLBACK (register_panel_action_inherit) },
	{ "Edit"				 , HB_STOCK_OPE_EDIT	 , N_("_Edit..."), NULL, N_("Edit the active transaction"),	G_CALLBACK (register_panel_action_edit) },
	{ "Reconcile"		 , "hb-ope-valid"			, N_("_Reconciled"), "r",		N_("Toggle reconciled status of active transactions"), G_CALLBACK (register_panel_action_reconcile) },
	{ "Remove"			 , HB_STOCK_OPE_DELETE , N_("_Remove..."), NULL,		N_("Remove the active transactions"), G_CALLBACK (register_panel_action_remove) },
	{ "Template"    	, GTK_STOCK_CONVERT		 , N_("Create template..."), NULL,		N_("Create template"), G_CALLBACK (register_panel_action_createtemplate) },

	{ "Assign"			 , "hb-assign-run"		 , N_("Auto. Assignments"), NULL,		N_("Run auto assignments"), G_CALLBACK (register_panel_action_assign) },
	{ "ExportQIF"		 , "hb-file-export"		, N_("Export QIF..."), NULL,		N_("Export as QIF"), G_CALLBACK (register_panel_action_exportqif) },
	{ "ExportCSV"		 , "hb-file-export"		, N_("Export CSV..."), NULL,		N_("Export as CSV"), G_CALLBACK (register_panel_action_exportcsv) },

};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"	<menubar name='MenuBar'>"

"		<menu action='AccountMenu'>"
"			<menuitem action='Close'/>"
"		</menu>"

"		<menu action='TransactionMenu'>"
"			<menuitem action='Add'/>"
"			<menuitem action='Inherit'/>"
"			<menuitem action='Edit'/>"
"			<menuitem action='Remove'/>"
"				<separator/>"
"			<menuitem action='Reconcile'/>"
"				<separator/>"
"			<menuitem action='Template'/>"
"		</menu>"

"		<menu action='ActionsMenu'>"
"			<menuitem action='Assign'/>"
"				<separator/>"
"			<menuitem action='ExportQIF'/>"
"			<menuitem action='ExportCSV'/>"
"		</menu>"

"		<menu action='ToolsMenu'>"
"			<menuitem action='Filter'/>"
"				<separator/>"
"		</menu>"
"	</menubar>"

"	<toolbar name='TxnBar'>"
"		<toolitem action='Reconcile'/>"
"		<toolitem action='Add'/>"
"		<toolitem action='Inherit'/>"
"		<toolitem action='Edit'/>"
"		<toolitem action='Remove'/>"
"			<separator/>"
"		<toolitem action='Template'/>"
"	</toolbar>"
"	<toolbar name='ToolBar'>"
"		<toolitem action='Filter'/>"
"		<toolitem action='Assign'/>"
"			<separator/>"
"		<toolitem action='ExportQIF'/>"
"		<toolitem action='ExportCSV'/>"
"	</toolbar>"
"</ui>";





GtkWidget *register_panel_window_new(guint32 accnum, Account *acc)
{
struct register_panel_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainbox, *hbox, *hbox2, *align, *sw;
GtkWidget *treeview, *label, *widget;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	DB( g_print("\n[account] create_register_panel_window\n") );

	data = g_malloc0(sizeof(struct register_panel_data));
	if(!data) return NULL;

	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

		/* create window, etc */
		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//debug
	data->wintitle = NULL;
	data->accnum = accnum;
	data->acc = acc;

	/* set transaction edit mutex */
	if(data->acc)
		data->acc->window = GTK_WINDOW(window);

	//g_free(data->wintitle);
	data->wintitle = g_strdup_printf("%s - HomeBank", data->acc->name);
	gtk_window_set_title (GTK_WINDOW (window), data->wintitle);

	// connect our dispose function
		g_signal_connect (window, "delete_event",
		G_CALLBACK (register_panel_dispose), (gpointer)data);

	// connect our dispose function
		g_signal_connect (window, "destroy",
		G_CALLBACK (register_panel_destroy), (gpointer)data);

	// connect our dispose function
		g_signal_connect (window, "configure-event",
		G_CALLBACK (register_panel_getgeometry), (gpointer)data);




	//gtk_window_set_icon_from_file(GTK_WINDOW (WI_account), "./pixmaps/.png", NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", window, data) );

	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_show.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_SHOW );


#if UI == 1
	//start test uimanager

		actions = gtk_action_group_new ("Account");

				//as we use gettext
				gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);


		DB( g_print(" - add actions: %p user_data: %p\n", actions, data) );
		gtk_action_group_add_actions (actions, entries, n_entries, data);

		/* set which action should have priority in the toolbar */
		action = gtk_action_group_get_action(actions, "Add");
		g_object_set(action, "is_important", TRUE, "short_label", _("Add"), NULL);

		action = gtk_action_group_get_action(actions, "Inherit");
		g_object_set(action, "is_important", TRUE, "short_label", _("Inherit"), NULL);

		action = gtk_action_group_get_action(actions, "Edit");
		g_object_set(action, "is_important", TRUE, "short_label", _("Edit"), NULL);

		action = gtk_action_group_get_action(actions, "Filter");
		g_object_set(action, "is_important", TRUE, "short_label", _("Filter"), NULL);

		action = gtk_action_group_get_action(actions, "Reconcile");
		g_object_set(action, "is_important", TRUE, "short_label", _("Reconcile"), NULL);


		ui = gtk_ui_manager_new ();

		DB( g_print(" - insert action group:\n") );
		gtk_ui_manager_insert_action_group (ui, actions, 0);

			GtkAccelGroup *ag = gtk_ui_manager_get_accel_group (ui);

			DB( g_print(" - add_accel_group actions=%x, ui=%x, ag=%x\n", (gint)actions, (gint)ui, (gint)ag) );

			gtk_window_add_accel_group (GTK_WINDOW (window), ag);

		DB( g_print(" - add ui from string:\n") );
	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}

	data->ui = ui;
	data->actions = actions;
#endif

	mainbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainbox);

	widget = gtk_ui_manager_get_widget (ui, "/MenuBar");
	//data->menu = widget;
	gtk_box_pack_start (GTK_BOX (mainbox), widget, FALSE, FALSE, 0);

	hbox  = gtk_hbox_new (FALSE, 3*HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

	// account name (+ balance)
	align = gtk_alignment_new(0.0, 0.5, 0.0, 1.0);
	gtk_alignment_set_padding(GTK_ALIGNMENT(align), 0, 0, 0, 4*HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), align, TRUE, TRUE, 0);

	label = gtk_label_new(data->acc->name);
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_LARGE, -1);
	gtk_container_add (GTK_CONTAINER (align), label);

	/* balances area */
	hbox2 = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_balance[2] = label;
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("Future:"));
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_balance[1] = label;
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("Today:"));
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_balance[0] = label;
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	label = gtk_label_new(_("Bank:"));
	gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 0);


	//quick search
	widget = gtk_entry_new ();
	data->ST_search = widget;
	gtk_entry_set_icon_from_stock (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, GTK_STOCK_FIND);
	gtk_entry_set_icon_from_stock (GTK_ENTRY (widget), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
	gtk_widget_set_size_request(widget, 240, -1);
	gtk_box_pack_end (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	g_signal_connect (widget, "activate", G_CALLBACK (quick_search_activate_cb), data);
	data->handler_id[HID_SEARCH] = g_signal_connect (widget, "notify::text", G_CALLBACK (quick_search_text_changed_cb), data);
	g_signal_connect (widget, "icon-press", G_CALLBACK (quick_search_icon_press_cb), data);

	
	// windows interior
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING/2);

	gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

	label = make_label(_("_Range:"), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_range = make_daterange(label, TRUE);
		gtk_box_pack_start (GTK_BOX (hbox), data->CY_range, FALSE, FALSE, 0);

	label = make_label(_("_Type:"), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_type = make_cycle(label, CYA_FLT_TYPE);
		gtk_box_pack_start (GTK_BOX (hbox), data->CY_type, FALSE, FALSE, 0);

	label = make_label(_("_Status:"), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_status = make_cycle(label, CYA_FLT_STATUS);
		gtk_box_pack_start (GTK_BOX (hbox), data->CY_status, FALSE, FALSE, 0);

	widget = gtk_button_new_with_mnemonic (_("Reset _Filter"));
	data->BT_reset = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	//TRANSLATORS: this is for Euro specific users, a toggle to display in 'Minor' currency
	widget = gtk_check_button_new_with_mnemonic (_("_Minor currency"));
	data->CM_minor = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	label = gtk_label_new(NULL);
	data->TX_selection = label;
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
	gtk_box_pack_end (GTK_BOX (hbox), label, TRUE, TRUE, 0);


	/*
	label = make_label(_("_Month:"), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->CY_month = make_cycle(label, CYA_SELECT);
		gtk_box_pack_start (GTK_BOX (hbox), data->CY_month, FALSE, FALSE, 0);

	label = make_label(_("_Year:"), 1.0, 0.5);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data->NB_year = make_year(label);
		gtk_box_pack_start (GTK_BOX (hbox), data->NB_year, FALSE, FALSE, 0);
	*/

	//list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	treeview = (GtkWidget *)create_list_transaction(TRN_LIST_TYPE_BOOK, PREFS->lst_ope_columns);
	data->LV_ope = treeview;
	gtk_container_add (GTK_CONTAINER (sw), treeview);
	gtk_box_pack_start (GTK_BOX (mainbox), sw, TRUE, TRUE, 0);

	/* toolbars */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

	widget = gtk_ui_manager_get_widget (ui, "/TxnBar");
	data->TB_bar = widget;
	//gtk_widget_set_halign (widget, GTK_ALIGN_START);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	widget = gtk_ui_manager_get_widget (ui, "/ToolBar");
	data->TB_tools = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

    #ifdef G_OS_WIN32
    if(PREFS->toolbar_style == 0)
    {
        gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
        gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_tools));
    }
    else
    {
        gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);
        gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_tools), PREFS->toolbar_style-1);
    }
    #endif

	//todo: should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_ope))), "minor", data->CM_minor);

	// connect signals
	data->handler_id[HID_RANGE]	= g_signal_connect (data->CY_range , "changed", G_CALLBACK (register_panel_cb_filter_daterange), NULL);
	data->handler_id[HID_TYPE]	 = g_signal_connect (data->CY_type	, "changed", G_CALLBACK (register_panel_cb_filter_type), NULL);
	data->handler_id[HID_STATUS] = g_signal_connect (data->CY_status, "changed", G_CALLBACK (register_panel_cb_filter_status), NULL);

	g_signal_connect (data->BT_reset , "clicked", G_CALLBACK (register_panel_cb_filter_reset), NULL);

	g_signal_connect (data->CM_minor , "toggled", G_CALLBACK (register_panel_toggle_minor), NULL);

	//g_signal_connect (GTK_TREE_VIEW(treeview), "cursor-changed", G_CALLBACK (register_panel_update), (gpointer)2);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (register_panel_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(treeview), "row-activated", G_CALLBACK (register_panel_onRowActivated), GINT_TO_POINTER(2));


//todo: test context menu
	/*
	menu = gtk_menu_new();
	menu_items = gtk_ui_manager_get_widget (ui, "/MenuBar/TransactionMenu/Add");

	menu_items = gtk_menu_item_new_with_label ("test");
	gtk_widget_show(menu_items);
	gtk_menu_shell_append (GTK_MENU (menu), menu_items);

	//todo: debug test
	g_signal_connect (treeview, "button-press-event", G_CALLBACK (listview_context_cb),
		// todo: here is not a GtkMenu but GtkImageMenuItem...
		menu
		//gtk_ui_manager_get_widget (ui, "/MenuBar")
	);
	*/

	//setup, init and show window
	wg = &PREFS->acc_wg;
	if(wg->s == 0)
	{
		gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
		gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);
	}
	else
		gtk_window_maximize(GTK_WINDOW(window));
	
	gtk_widget_show_all (window);

	/* make sure splash is up */
	while (gtk_events_pending ())
		gtk_main_iteration ();

	/* setup to moove later */
	data->filter = da_filter_malloc();
	DB( g_print(" - filter ok %x\n", (gint)data->filter) );


	return window;
}
