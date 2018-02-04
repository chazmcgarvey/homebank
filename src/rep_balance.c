/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2018 Maxime DOYEN
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

#include "rep_balance.h"

#include "list_operation.h"
#include "gtk-chart.h"
#include "gtk-dateentry.h"

#include "ui-account.h"
#include "dsp_mainwindow.h"
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


/* prototypes */
static void repbalance_action_viewlist(GtkAction *action, gpointer user_data);
static void repbalance_action_viewline(GtkAction *action, gpointer user_data);
static void repbalance_action_detail(GtkAction *action, gpointer user_data);
static void repbalance_action_refresh(GtkAction *action, gpointer user_data);
static void repbalance_update_daterange(GtkWidget *widget, gpointer user_data);
static void repbalance_update_detail(GtkWidget *widget, gpointer user_data);
static void repbalance_toggle_detail(GtkWidget *widget, gpointer user_data);
static void repbalance_detail(GtkWidget *widget, gpointer user_data);
static void repbalance_sensitive(GtkWidget *widget, gpointer user_data);
/* prototypes */
static void repbalance_date_change(GtkWidget *widget, gpointer user_data);
static void repbalance_range_change(GtkWidget *widget, gpointer user_data);
static void repbalance_update_info(GtkWidget *widget, gpointer user_data);
static void repbalance_toggle_minor(GtkWidget *widget, gpointer user_data);
static void repbalance_compute(GtkWidget *widget, gpointer user_data);
static void repbalance_setup(struct repbalance_data *data, guint32 accnum);
static gboolean repbalance_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static GtkWidget *create_list_repbalance(void);
static void ui_repbalance_list_set_cur(GtkTreeView *treeview, guint32 kcur);

//todo amiga/linux
//prev
//next

static GtkRadioActionEntry radio_entries[] = {
  { "List"    , ICONNAME_HB_VIEW_LIST  , N_("List")   , NULL,    N_("View results as list")  , 0 },
  { "Line"    , ICONNAME_HB_VIEW_LINE  , N_("Line")   , NULL,    N_("View results as lines") , 1 },
//  { "Column"  , ICONNAME_HB_VIEW_COLUMN, N_("Column") , NULL,    N_("View results as column"), 2 },
};
static guint n_radio_entries = G_N_ELEMENTS (radio_entries);

static GtkActionEntry entries[] = {
  { "Refresh" , ICONNAME_REFRESH   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (repbalance_action_refresh) },
};

static guint n_entries = G_N_ELEMENTS (entries);

static GtkToggleActionEntry toggle_entries[] = {
  { "Detail", ICONNAME_HB_OPE_SHOW,                    /* name, icon-name */
     N_("Detail"), NULL,                    /* label, accelerator */
    N_("Toggle detail"),                                    /* tooltip */
    G_CALLBACK (repbalance_action_detail),
    FALSE },                                    /* is_active */
};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);


static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Line'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"      <separator/>"
"    <toolitem action='Refresh'/>"
"  </toolbar>"
"</ui>";


//extern gchar *CYA_FLT_SELECT[];


/* action functions -------------------- */
static void repbalance_action_viewlist(GtkAction *action, gpointer user_data)
{
struct repbalance_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	repbalance_sensitive(data->window, NULL);
}

static void repbalance_action_viewline(GtkAction *action, gpointer user_data)
{
struct repbalance_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	repbalance_sensitive(data->window, NULL);
}


static void ui_repbalance_action_mode (GtkRadioAction *action, GtkRadioAction *current, gpointer user_data)
{
gint value;

	value = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(action));
	switch( value )
	{
		case 0:
			repbalance_action_viewlist(GTK_ACTION(action), user_data);
			break;
		case 1:
			repbalance_action_viewline(GTK_ACTION(action), user_data);
			break;
	}
}

static void repbalance_action_detail(GtkAction *action, gpointer user_data)
{
struct repbalance_data *data = user_data;

	repbalance_toggle_detail(data->window, NULL);
}

static void repbalance_action_refresh(GtkAction *action, gpointer user_data)
{
struct repbalance_data *data = user_data;

	repbalance_compute(data->window, NULL);
}



/* ======================== */






static void repbalance_date_change(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;

	DB( g_print(" \n[repbalance] date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_date_entry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_date_entry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);
	
	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPBALANCE_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), FLT_RANGE_OTHER);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPBALANCE_RANGE]);


	repbalance_compute(widget, NULL);
	repbalance_update_daterange(widget, NULL);

}


static void repbalance_update_quickdate(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;

	DB( g_print(" \n[repbalance] update quickdate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPBALANCE_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPBALANCE_MAXDATE]);
	
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
	
	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPBALANCE_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPBALANCE_MAXDATE]);

}



static void repbalance_range_change(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
gint range;

	DB( g_print(" \n[repbalance] range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range, data->accnum);

		repbalance_update_quickdate(widget, NULL);

		repbalance_compute(widget, NULL);
		repbalance_update_daterange(widget, NULL);
	}
	
}

static void repbalance_update_daterange(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
gchar *daterange;

	DB( g_print(" \n[repbalance] update daterange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	daterange = filter_daterange_text_get(data->filter);
	gtk_label_set_markup(GTK_LABEL(data->TX_daterange), daterange);
	g_free(daterange);
}


static void repbalance_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key = -1;

	DB( g_print(" \n[repbalance] selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_OVER_DATE, &key, -1);
	}

	DB( g_print(" - active is %d\n", key) );

	repbalance_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	repbalance_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


/*
** update sensitivity
*/
static void repbalance_sensitive(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
GtkAction *action;
gboolean visible;
gint page;

	DB( g_print(" \n[repbalance] sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	action = gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail");
	visible = page == 0 ? TRUE : FALSE;
	gtk_action_set_visible (action, visible);
	//sensitive = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);
	//gtk_action_set_sensitive(action, sensitive);

	visible = page == 0 ? FALSE : TRUE;
	hb_widget_visible(data->LB_zoomx, visible);
	hb_widget_visible(data->RG_zoomx, visible);

}


static void repbalance_update_info(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
gchar *info;
gchar   buf[128];
Account *acc;

	DB( g_print(" \n[repbalance] update info\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	gboolean selectall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_selectall));

	guint32 acckey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	DB( g_print(" acc key = %d\n", acckey) );

	acc = da_acc_get(acckey);
	// 1635857 
	if( acc != NULL )
	{
		gboolean minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

		hb_strfmon(buf, 127, data->minimum, selectall ? GLOBALS->kcur : acc->kcur, minor);

		////TRANSLATORS: count of transaction in balancedrawn / count of total transaction under abalancedrawn amount threshold
		info = g_strdup_printf(_("%d/%d under %s"), data->nbbalance, data->nbope, buf);
		gtk_label_set_text(GTK_LABEL(data->TX_info), info);
		g_free(info);
	}
	
}


static void repbalance_detail(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
guint active = GPOINTER_TO_INT(user_data);
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;
guint32 acckey;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print(" \n[repbalance] detail\n") );

	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if(data->detail)
	{
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */


		gboolean selectall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_selectall));

		// get the account key
		acckey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

		/* fill in the model */
		list = g_list_first(data->ope_list);
		while (list != NULL)
		{
		Transaction *ope = list->data;

			//DB( g_print(" get %s\n", ope->ope_Word) );

			//filter here
			if( ope->date == active && (ope->kacc == acckey || selectall) )
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		 		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_DSPOPE_DATAS, ope,
							-1);
			}
			list = g_list_next(list);
		}

		/* Re-attach model to view */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), model);
		g_object_unref(model);

		gtk_tree_view_columns_autosize( GTK_TREE_VIEW(data->LV_detail) );

	}
}


static void repbalance_detail_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct repbalance_data *data;
Transaction *active_txn;
gboolean result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[repbalance] A detail row has been double-clicked!\n") );

	active_txn = list_txn_get_active_transaction(GTK_TREE_VIEW(data->LV_detail));
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
			repbalance_compute (data->window, NULL);
		}

		da_transaction_free (old_txn);
	}
}



static void repbalance_update_detail(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(data->detail)
	{
	GtkTreeSelection *treeselection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	guint key;

		treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_report));

		if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
		{
			gtk_tree_model_get(model, &iter, LST_OVER_DATE, &key, -1);

			DB( g_print(" - active is %d\n", key) );

			repbalance_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
		}



		gtk_widget_show(data->GR_detail);
	}
	else
		gtk_widget_hide(data->GR_detail);
}


static void repbalance_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->detail ^= 1;

	DB( g_print(" \n[repbalance] toggledetail to %d\n", (int)data->detail) );

	repbalance_update_detail(widget, user_data);

}


static void repbalance_zoomx_callback(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
gdouble value;

	DB( g_print("(statistic) zoomx\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_range_get_value(GTK_RANGE(data->RG_zoomx));

	DB( g_print(" + scale is %f\n", value) );

	gtk_chart_set_barw(GTK_CHART(data->RE_line), value);

}


static void repbalance_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;

	DB( g_print(" \n[repbalance] toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	repbalance_update_info(widget,NULL);

	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	gtk_chart_show_minor(GTK_CHART(data->RE_line), GLOBALS->minor);
}


static void repbalance_compute_full_datas(guint32 selkey, gboolean selectall, struct repbalance_data *data)
{
GList *list;
GList *lst_acc, *lnk_acc;

	DB( g_print(" \n[repbalance] compute_full\n") );

	g_list_free(data->ope_list);
	data->ope_list = hbfile_transaction_get_all();

	if(g_list_length(data->ope_list) == 0) return;

	Transaction *omin = g_list_first(data->ope_list)->data;
	Transaction *omax = g_list_last(data->ope_list)->data;

	// total days in the hbfile
	data->n_result = omax->date - omin->date;

	DB( g_print(" - %d days in slice\n", data->n_result) );

	data->tmp_income  = g_malloc0((data->n_result+2) * sizeof(gdouble));
	data->tmp_expense = g_malloc0((data->n_result+2) * sizeof(gdouble));

	// to store that initial balance was affected
	gboolean *accounts = g_malloc0((da_acc_get_max_key()+2) * sizeof(gboolean));

	if(data->tmp_income && data->tmp_expense)
	{
	gdouble trn_amount;

		/* account initial amount */
		lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
		lnk_acc = g_list_first(lst_acc);
		while (lnk_acc != NULL)
		{
		Account *acc = lnk_acc->data;

			//#1674045 ony rely on nosummary
			//if( (acc->flags & (AF_CLOSED|AF_NOREPORT)) )
			if( (acc->flags & (AF_NOREPORT)) )
				goto next_acc;

			trn_amount = 0.0;
			if(selectall)
				trn_amount = hb_amount_base(acc->initial, acc->kcur);
			else
				if( selkey == acc->key )
					trn_amount = acc->initial;

			if(trn_amount < 0)
				data->tmp_expense[0] += trn_amount;
			else
				data->tmp_income[0] += trn_amount;

			DB( g_print(" - stored initial %.2f for account %d:%s\n", trn_amount, acc->key, acc->name) );

		next_acc:
			lnk_acc = g_list_next(lnk_acc);
		}
		g_list_free(lst_acc);

		/* compute the balance */
		list = g_list_first(data->ope_list);
		while (list != NULL)
		{
		gint pos;
		Transaction *ope = list->data;

			if(selkey == ope->kacc || selectall == TRUE)
			{
			Account *acc = da_acc_get(ope->kacc);

				if( acc != NULL )
				{
					pos = ope->date - omin->date;

					// deal with account initial balance
					/*if(accounts[ope->kacc] == 0)
					{
						if(selectall)
							trn_amount = hb_amount_base(acc->initial, acc->kcur);
						else
							trn_amount = acc->initial;

						if(trn_amount < 0)
							data->tmp_expense[pos] += trn_amount;
						else
							data->tmp_income[pos] += trn_amount;

						DB( g_print(" - stored initial %.2f for account %d\n", trn_amount, ope->kacc) );

						accounts[ope->kacc] = 1;
					}*/

					if(selectall)
						trn_amount = hb_amount_base(ope->amount, acc->kcur);
					else
						trn_amount = ope->amount;

					//deal with transactions
					if(trn_amount < 0)
						data->tmp_expense[pos] += trn_amount;
					else
						data->tmp_income[pos] += trn_amount;
				}
			}

			list = g_list_next(list);
		}

	}

	g_free(accounts);

}


static void repbalance_compute(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
guint32 acckey, i;
gboolean selectall, eachday;
Account *acc;

	DB( g_print(" \n[repbalance] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selectall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_selectall));
	eachday = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_eachday));

	// get the account key
	acckey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	DB( g_print(" acc key = %d\n", acckey) );

	data->nbope = 0;
	data->nbbalance = 0;
	data->minimum = 0;
	data->accnum = 0;

	// for a single account
	if(!selectall)
	{
		acc = da_acc_get(acckey);
		if(acc != NULL)
		{
			data->minimum = acc->minimum;
			data->accnum = acc->key;
			ui_repbalance_list_set_cur(GTK_TREE_VIEW(data->LV_report), acc->kcur);
			gtk_chart_set_currency(GTK_CHART(data->RE_line), acc->kcur);
		}
	}
	else
	{
	
		ui_repbalance_list_set_cur(GTK_TREE_VIEW(data->LV_report), GLOBALS->kcur);
		gtk_chart_set_currency(GTK_CHART(data->RE_line), GLOBALS->kcur);
	}

	//to remove > 5.0.2
	//#1715532 5.0.5: no... but only selectall
	if(selectall == TRUE)
	{
		filter_preset_daterange_set(data->filter, data->filter->range, data->accnum);
		repbalance_update_quickdate(widget, NULL);
	}

	repbalance_compute_full_datas(acckey, selectall, data);

	/* do nothing if no transaction */
	if(g_list_length(data->ope_list) == 0) return;


	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

	Transaction *omin = g_list_first(data->ope_list)->data;
	gdouble balance = 0;

	for(i=0;i<=data->n_result;i++)
	{
	gboolean is_balance = FALSE;
	GDate *date;
	gchar buf[256];
	guint32 posdate;

		posdate = omin->date + i;
		
		//DB( g_print("omin->date=%d posdate=%d\n", omin->date, posdate) );
		

		balance += data->tmp_expense[i];
		balance += data->tmp_income[i];

		if(!eachday && data->tmp_expense[i] == 0 && data->tmp_income[i] == 0)
			continue;

		if( (posdate >= data->filter->mindate) && (posdate <= data->filter->maxdate) )
		{
			if(!selectall)
				is_balance = (balance < data->minimum) ? TRUE : FALSE;

			date = g_date_new_julian (posdate);
			g_date_strftime (buf, 256-1, PREFS->date_format, date);
			g_date_free(date);

			/* column 0: pos (gint) */
			/* not used: column 1: key (gint) */
			/* column 2: name (gchar) */
			/* column x: values (double) */

			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_OVER_OVER, is_balance,
				LST_OVER_DATE, posdate,
				LST_OVER_DATESTR, buf,
				LST_OVER_EXPENSE, data->tmp_expense[i],
				LST_OVER_INCOME, data->tmp_income[i],
				LST_OVER_BALANCE, balance,
				-1);
			if(is_balance == TRUE)
				data->nbbalance++;

			data->nbope++;

		}

	}

	g_free(data->tmp_expense);
	g_free(data->tmp_income);

	repbalance_update_info(widget, NULL);

	gtk_chart_show_legend(GTK_CHART(data->RE_line), FALSE, FALSE);
	gtk_chart_show_xval(GTK_CHART(data->RE_line), TRUE);
	gtk_chart_set_overdrawn(GTK_CHART(data->RE_line), data->minimum);
	gtk_chart_show_overdrawn(GTK_CHART(data->RE_line), !selectall);

	gboolean visible = selectall ? FALSE : TRUE;
	gtk_widget_set_visible (GTK_WIDGET(data->TX_info), visible);

	/* Re-attach model to view */
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
	g_object_unref(model);

	/* update bar chart */
	//DB( g_print(" set bar to %d\n\n", LST_STAT_EXPENSE+tmpkind) );
	gtk_chart_set_datas(GTK_CHART(data->RE_line), model, LST_OVER_BALANCE, NULL, NULL);
	//gtk_chart_set_line_datas(GTK_CHART(data->RE_line), model, LST_OVER_BALANCE, LST_OVER_DATE);


}


static void repbalance_toggle_selectall(GtkWidget *widget, gpointer user_data)
{
struct repbalance_data *data;
gboolean selectall;

	DB( g_print(" \n[repbalance] toggle selectall\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selectall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_selectall));

	gtk_widget_set_sensitive(GTK_WIDGET(data->PO_acc), selectall^1);

	repbalance_compute(widget, data);

}


static void repbalance_setup(struct repbalance_data *data, guint32 accnum)
{
	DB( g_print(" \n[repbalance] setup\n") );

	data->ope_list = NULL;

	data->filter = da_filter_malloc();
	filter_default_all_set(data->filter);

	data->accnum = accnum;
	filter_preset_daterange_set(data->filter, PREFS->date_range_rep, data->accnum);
	repbalance_update_quickdate(data->window, NULL);
	
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_REPORT);
	if( accnum )
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_acc), accnum);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), 0);

}


static gboolean repbalance_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repbalance_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print(" \n[repbalance] dispose\n") );

	g_list_free (data->ope_list);

	da_filter_free(data->filter);

	g_free(data);

	//store position and size
	wg = &PREFS->ove_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);

	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

	return FALSE;
}


// the window creation
GtkWidget *repbalance_window_new(gint accnum)
{
struct repbalance_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
//GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct repbalance_data));
	if(!data) return NULL;

	DB( g_print(" \n[repbalance] new\n") );

	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", window, data) );

	gtk_window_set_title (GTK_WINDOW (window), _("Balance report"));

	//set the window icon
	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_REP_BALANCE);

	//window contents
	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//control part
	table = gtk_grid_new ();
	gtk_widget_set_hexpand (GTK_WIDGET(table), FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_SMALL);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);


	row = 0;
	label = make_label_group(_("Display"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("A_ccount:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Select _all"));
	data->CM_selectall = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Each _day"));
	//gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(widget), TRUE);
	data->CM_eachday = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Euro _minor"));
	data->CM_minor = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Zoom X:"));
	data->LB_zoomx = label;
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_scale(label);
	data->RG_zoomx = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_grid_attach (GTK_GRID (table), widget, 0, row, 3, 1);

	row++;
	label = make_label_group(_("Date filter"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("_Range:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	data->CY_range = make_daterange(label, FALSE);
	gtk_grid_attach (GTK_GRID (table), data->CY_range, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_From:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	data->PO_mindate = gtk_date_entry_new();
	gtk_grid_attach (GTK_GRID (table), data->PO_mindate, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_To:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	data->PO_maxdate = gtk_date_entry_new();
	gtk_grid_attach (GTK_GRID (table), data->PO_maxdate, 2, row, 1, 1);


	//part: info + report
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	//ui manager
	actions = gtk_action_group_new ("default");

	//as we use gettext
   	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	// data to action callbacks is set here (data)
	gtk_action_group_add_radio_actions (actions, radio_entries, n_radio_entries, 0, G_CALLBACK(ui_repbalance_action_mode), data);

	gtk_action_group_add_actions (actions, entries, n_entries, data);

	gtk_action_group_add_toggle_actions (actions,
					   toggle_entries, n_toggle_entries,
					   data);


	/* set which action should have priority in the toolbar */
	//action = gtk_action_group_get_action(actions, "List");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Line");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Detail");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Refresh");
	//g_object_set(action, "is_important", TRUE, NULL);



	ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building UI failed: %s", error->message);
		g_error_free (error);
	}

	data->ui = ui;
	data->actions = actions;

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);


	label = gtk_label_new(NULL);
	data->TX_info = label;
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	widget = make_label(NULL, 0.5, 0.5);
	gimp_label_set_attributes (GTK_LABEL (widget), PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL, -1);
	data->TX_daterange = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);


	notebook = gtk_notebook_new();
	data->GR_result = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);

    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

	//page: list
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, NULL);

	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	treeview = create_list_repbalance();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
	//gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = widget;
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (widget), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);

    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//page: 2d lines
	widget = gtk_chart_new(CHART_TYPE_LINE);
	data->RE_line = widget;
	//gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);




	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);



	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);





	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repbalance_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repbalance_toggle_minor), NULL);


    data->handler_id[HID_REPBALANCE_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repbalance_date_change), (gpointer)data);
    data->handler_id[HID_REPBALANCE_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repbalance_date_change), (gpointer)data);

	data->handler_id[HID_REPBALANCE_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repbalance_range_change), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (repbalance_selection), NULL);

	//setup, init and show window
	repbalance_setup(data, accnum);

	g_signal_connect (data->CM_selectall, "toggled", G_CALLBACK (repbalance_toggle_selectall), NULL);
	g_signal_connect (data->CM_eachday, "toggled", G_CALLBACK (repbalance_compute), NULL);


	//let this here or the setup trigger a compute...
	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (repbalance_compute), NULL);

	g_signal_connect (data->RG_zoomx, "value-changed", G_CALLBACK (repbalance_zoomx_callback), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_detail), "row-activated", G_CALLBACK (repbalance_detail_onRowActivated), NULL);


	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);


	//setup, init and show window
	wg = &PREFS->ove_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);
	data->detail = 0;


	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	repbalance_sensitive(window, NULL);
	repbalance_update_detail(window, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_rep);


	return(window);
}

/*
** ============================================================================
*/


static void repbalance_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gchar *datestr;
gboolean is_balance;
gchar *color;
gint weight;

	gtk_tree_model_get(model, iter,
		LST_OVER_DATESTR, &datestr,
		LST_OVER_OVER, &is_balance,
		-1);

	color = NULL;
	weight = PANGO_WEIGHT_NORMAL;

	if(is_balance==TRUE)
	{
		if(PREFS->custom_colors == TRUE)
			color = PREFS->color_warn;

		weight = PANGO_WEIGHT_BOLD;
	}

	g_object_set(renderer,
		"weight", weight,
		"foreground",  color,
		"text", datestr,
		NULL);

	//leak
	g_free(datestr);
}


static void repbalance_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gboolean is_balance;
gchar *color;
gint weight;
guint32 kcur = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(gtk_tree_view_column_get_tree_view(col)), "kcur_data"));


	//get datas
	gtk_tree_model_get(model, iter,
		LST_OVER_OVER, &is_balance,
		GPOINTER_TO_INT(user_data), &value,
		-1);

	//fix: 400483
	if( value == 0.0 )
		g_object_set(renderer, "text", NULL, NULL);
	else
	{
		hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur, GLOBALS->minor);

		color = NULL;
		weight = PANGO_WEIGHT_NORMAL;


		if(value != 0.0 && PREFS->custom_colors == TRUE)
			color = (value > 0.0) ? PREFS->color_inc : PREFS->color_exp;

		if(is_balance==TRUE)
		{
			if(PREFS->custom_colors == TRUE)
				color = PREFS->color_warn;

			weight = PANGO_WEIGHT_BOLD;
		}

		g_object_set(renderer,
			"weight", weight,
			"foreground",  color,
			"text", buf,
			NULL);
	}

}

static GtkTreeViewColumn *amount_list_repbalance_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repbalance_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}


static void ui_repbalance_list_set_cur(GtkTreeView *treeview, guint32 kcur)
{
	g_object_set_data(G_OBJECT(treeview), "kcur_data", GUINT_TO_POINTER(kcur));
}


/*
** create our statistic list
*/
static GtkWidget *create_list_repbalance(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_OVER,
		G_TYPE_BOOLEAN,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);

	/* column debug balance */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "debug balance");
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_OVER_OVER);
*/

	/* column date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_OVER_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repbalance_date_cell_data_function, NULL, NULL);


	/* column: Expense */
	column = amount_list_repbalance_column(_("Expense"), LST_OVER_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = amount_list_repbalance_column(_("Income"), LST_OVER_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Balance */
	column = amount_list_repbalance_column(_("Balance"), LST_OVER_BALANCE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);



	return(view);
}
