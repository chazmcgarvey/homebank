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


#include "homebank.h"

#include "rep_budget.h"

#include "list_operation.h"
#include "gtk-chart-stack.h"
#include "gtk-dateentry.h"

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


static void repbudget_action_viewlist(GtkAction *action, gpointer user_data);
static void repbudget_action_viewbar(GtkAction *action, gpointer user_data);
static void repbudget_action_detail(GtkAction *action, gpointer user_data);
static void repbudget_action_refresh(GtkAction *action, gpointer user_data);
static void repbudget_action_export(GtkAction *action, gpointer user_data);

static void repbudget_date_change(GtkWidget *widget, gpointer user_data);
static void repbudget_range_change(GtkWidget *widget, gpointer user_data);
static void repbudget_toggle_detail(GtkWidget *widget, gpointer user_data);
static void repbudget_detail(GtkWidget *widget, gpointer user_data);
static void repbudget_compute(GtkWidget *widget, gpointer user_data);
static void repbudget_update_total(GtkWidget *widget, gpointer user_data);
static void repbudget_sensitive(GtkWidget *widget, gpointer user_data);
static void repbudget_toggle(GtkWidget *widget, gpointer user_data);
static GtkWidget *create_list_budget(void);
static void repbudget_update_detail(GtkWidget *widget, gpointer user_data);
static void repbudget_update_daterange(GtkWidget *widget, gpointer user_data);
static void repbudget_export_csv(GtkWidget *widget, gpointer user_data);

gchar *CYA_BUDGSELECT[] = { N_("Category"), N_("Subcategory"), NULL };

gchar *CYA_KIND[] = { N_("Exp. & Inc."), N_("Expense"), N_("Income"), NULL };

gchar *CYA_BUDGETSELECT[] = { N_("Spent & Budget"), N_("Spent"), N_("Budget"), N_("Result"), NULL };


//extern gchar *CYA_FLT_SELECT[];

static GtkActionEntry entries[] = {
  { "List"    , ICONNAME_HB_VIEW_LIST , N_("List")  , NULL,    N_("View results as list"), G_CALLBACK (repbudget_action_viewlist) },
  { "Stack"     , ICONNAME_HB_VIEW_STACK, N_("Stack")   , NULL,    N_("View results as stack bars"), G_CALLBACK (repbudget_action_viewbar) },
  { "Refresh" , ICONNAME_REFRESH, N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (repbudget_action_refresh) },

  { "Export"  , ICONNAME_HB_FILE_EXPORT , N_("Export")  , NULL,   N_("Export as CSV"), G_CALLBACK (repbudget_action_export) },
};
static guint n_entries = G_N_ELEMENTS (entries);


static GtkToggleActionEntry toggle_entries[] = {
  { "Detail", ICONNAME_HB_OPE_SHOW,                    /* name, icon-name */
     N_("Detail"), NULL,                    /* label, accelerator */
    N_("Toggle detail"),                                    /* tooltip */
    G_CALLBACK (repbudget_action_detail),
    FALSE },                                    /* is_active */

};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);


static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Stack'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"      <separator/>"
"    <toolitem action='Refresh'/>"
"      <separator/>"
"    <toolitem action='Export'/>"
"  </toolbar>"
"</ui>";



/* action functions -------------------- */
static void repbudget_action_viewlist(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	repbudget_sensitive(data->window, NULL);
}

static void repbudget_action_viewbar(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	repbudget_sensitive(data->window, NULL);
}

static void repbudget_action_detail(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_toggle_detail(data->window, NULL);
}


static void repbudget_action_refresh(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_compute(data->window, NULL);
}

static void repbudget_action_export(GtkAction *action, gpointer user_data)
{
struct repbudget_data *data = user_data;

	repbudget_export_csv(data->window, NULL);
}

/* ======================== */


static gint getmonth(guint date)
{
GDate *date1;
gint month;

	date1 = g_date_new_julian(date);
	month = g_date_get_month(date1);

	/*#if MYDEBUG == 1
		g_print("\n[repbudget] getmonth\n");
		gchar buffer1[128];
		g_date_strftime (buffer1, 128-1, "%x", date1);
		g_print("  date is '%s', month=%d\n", buffer1, month);
	#endif*/

	g_date_free(date1);

	return(month);
}

static gint countmonth(guint32 mindate, guint32 maxdate)
{
GDate *date1, *date2;
gint nbmonth;

	date1 = g_date_new_julian(mindate);
	date2 = g_date_new_julian(maxdate);

	nbmonth = 0;
	while(g_date_compare(date1, date2) < 0)
	{
		nbmonth++;
		g_date_add_months(date1, 1);
	}

	g_date_free(date2);
	g_date_free(date1);

	return(nbmonth);
}

static gdouble budget_compute_result(gdouble budget, gdouble spent)
{
gdouble retval;

	//original formula
	//result = ABS(budget) - ABS(spent);

	retval = spent - budget;

	return retval;
}





static void repbudget_date_change(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	DB( g_print("\n[repbudget] date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_date_entry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_date_entry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPBUDGET_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), FLT_RANGE_OTHER);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPBUDGET_RANGE]);


	repbudget_compute(widget, NULL);
	repbudget_update_daterange(widget, NULL);

}


static void repbudget_range_change(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gint range;

	DB( g_print("\n[repbudget] range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));
	

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range, 0);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPBUDGET_MINDATE]);
		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPBUDGET_MAXDATE]);
		
		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPBUDGET_MINDATE]);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPBUDGET_MAXDATE]);

		repbudget_compute(widget, NULL);
		repbudget_update_daterange(widget, NULL);
	}
}


static void repbudget_update_daterange(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gchar *daterange;

	DB( g_print("\n[repbudget] update daterange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	daterange = filter_daterange_text_get(data->filter);
	gtk_label_set_markup(GTK_LABEL(data->TX_daterange), daterange);
	g_free(daterange);
}


static void repbudget_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[repbudget] toggle detail\n") );

	data->detail ^= 1;

	repbudget_update_detail(widget, user_data);

}


static void repbudget_detail_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct repbudget_data *data;
Transaction *active_txn;
gboolean result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[repbudget] A detail row has been double-clicked!\n") );

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
			repbudget_compute(data->window, NULL);
		}

		da_transaction_free (old_txn);
	}
}


static void repbudget_update_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[repbudget] update detail\n") );

	if(GTK_IS_TREE_VIEW(data->LV_report))
	{
		if(data->detail)
		{
		GtkTreeSelection *treeselection;
		GtkTreeModel *model;
		GtkTreeIter iter;
		guint key;

			treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_report));

			if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
			{
				gtk_tree_model_get(model, &iter, LST_BUDGET_KEY, &key, -1);

				//DB( g_print(" - active is %d\n", pos) );

				repbudget_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
			}


			gtk_widget_show(data->GR_detail);
		}
		else
			gtk_widget_hide(data->GR_detail);

		
	}
}


static void repbudget_export_csv(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gchar *filename = NULL;
GIOChannel *io;
gchar *outstr, *name;
gint tmpfor;

	DB( g_print("\n[repbudget] export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));

	name = g_strdup_printf("hb-budget_%s.csv", CYA_BUDGSELECT[tmpfor]);

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			// header
			outstr = g_strdup_printf("%s;%s;%s;%s;\n", _("Category"), _("Spent"), _("Budget"), _("Result"));
			g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			gchar *name, *status;
			gdouble spent, budget, result;

				gtk_tree_model_get (model, &iter,
					//LST_REPDIST_KEY, i,
					LST_BUDGET_NAME, &name,
					LST_BUDGET_SPENT, &spent,
					LST_BUDGET_BUDGET, &budget,
					LST_BUDGET_RESULT, &result,
				    LST_BUDGET_STATUS, &status,
					-1);

				outstr = g_strdup_printf("%s;%.2f;%.2f;%.2f;%s\n", name, spent, budget, result, status);
				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				DB( g_print("%s", outstr) );

				g_free(outstr);

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

		g_free( filename );
	}

	g_free(name);


}


static void repbudget_detail(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
guint active = GPOINTER_TO_INT(user_data);
GList *list;
guint tmpfor;
GtkTreeModel *model;
GtkTreeIter  iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[repbudget] detail\n") );

	if(data->detail)
	{
		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));

		/* fill in the model */
		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Account *acc;
		Transaction *ope = list->data;
		guint pos = 0;
		gboolean insert = FALSE;

			//DB( g_print(" get %s\n", ope->ope_Word) );

			acc = da_acc_get(ope->kacc);
			if(acc != NULL)
			{
				if((acc->flags & AF_NOBUDGET)) goto next1;
			}

			//filter here
			if( ope->flags & OF_SPLIT )
			{
			guint nbsplit = da_splits_count(ope->splits);
			Split *split;
			guint i;
			
				for(i=0;i<nbsplit;i++)
				{
					split = ope->splits[i];
					switch(tmpfor)
					{
						case BUDG_CATEGORY:
							{
							Category *catentry = da_cat_get(split->kcat);
								if(catentry)
									pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
							}
							break;
						case BUDG_SUBCATEGORY:
							pos = split->kcat;
							break;
					}
					
					if( pos == active )
					{	insert = TRUE; break; }
				}
			}
			else
			{
				switch(tmpfor)
				{
					case BUDG_CATEGORY:
						{
						Category *catentry = da_cat_get(ope->kcat);
							if(catentry)
								pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
						}
						break;
					case BUDG_SUBCATEGORY:
						pos = ope->kcat;
						break;
				}
				
				if( pos == active )
					insert = TRUE;
			}

			//insert
			if( insert == TRUE )
			{

		    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_DSPOPE_DATAS, ope,
					-1);
			}

			
next1:
			list = g_list_next(list);
		}

		/* Re-attach model to view */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), model);
		g_object_unref(model);

		gtk_tree_view_columns_autosize( GTK_TREE_VIEW(data->LV_detail) );

	}

}


static void repbudget_compute(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

gint tmpfor, tmpkind;
guint32 mindate, maxdate;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
guint n_result, id;
gdouble *tmp_spent, *tmp_budget;
gint nbmonth = 1;
gchar *title;

	DB( g_print("\n[repbudget] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor   = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpkind  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_kind));

	mindate = data->filter->mindate;
	maxdate = data->filter->maxdate;
	if(maxdate < mindate) return;

	g_queue_free (data->txn_queue);
	data->txn_queue = hbfile_transaction_get_partial_budget(data->filter->mindate, data->filter->maxdate);


	DB( g_print(" for=%d, kind=%d\n", tmpfor, tmpkind) );

	nbmonth = countmonth(mindate, maxdate);
	DB( g_print(" date: min=%d max=%d nbmonth=%d\n", mindate, maxdate, nbmonth) );

	n_result = da_cat_get_max_key();
	DB( g_print(" nbcat=%d\n", n_result) );

	/* allocate some memory */
	tmp_spent  = g_malloc0((n_result+1) * sizeof(gdouble));
	tmp_budget = g_malloc0((n_result+1) * sizeof(gdouble));

	if(tmp_spent && tmp_budget)
	{
	guint i = 0;
		/* compute the results */
		data->total_spent = 0.0;
		data->total_budget = 0.0;

		/* compute budget for each category */
		DB( g_print("\n+ compute budget\n") );

		//fixed #328034: here <=n_result
		for(i=1; i<=n_result; i++)
		{
		Category *entry;
		//gchar buffer[128];
		gint pos;

			entry = da_cat_get(i);
			if( entry == NULL)
				continue;

			DB( g_print(" category %d:'%s' issub=%d hasbudget=%d custom=%d\n", 
				entry->key, entry->name, (entry->flags & GF_SUB), (entry->flags & GF_BUDGET), (entry->flags & GF_CUSTOM)) );

			//debug
			#if MYDEBUG == 1
			gint k;
			g_print("    budget vector: ");
			for(k=0;k<13;k++)
				g_print( " %d:[%.2f]", k, entry->budget[k]);
			g_print("\n");
			#endif

			pos = 0;
			switch(tmpfor)
			{
				case BUDG_CATEGORY:
					{
					Category *catentry = da_cat_get(i);
						if(catentry)
							pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
					}
					break;
				case BUDG_SUBCATEGORY:
					pos = i;
					break;
			}

			// same value each month ?
			if(!(entry->flags & GF_CUSTOM))
			{
				DB( g_print("    - monthly %.2f\n", entry->budget[0]) );
				tmp_budget[pos] += entry->budget[0]*nbmonth;
			}
			//otherwise	sum each month from mindate month
			else
			{
			gint month = getmonth(mindate);
			gint j;

				DB( g_print("    - custom each month for %d months\n", nbmonth) );
				for(j=0;j<nbmonth;j++) {
					DB( g_print("      j=%d month=%d budg=%.2f\n", j, month, entry->budget[month]) );
					tmp_budget[pos] += entry->budget[month];
					month++;
					if(month > 12) {
						month = 1;
					}
				}
			}

			//debug
			#if MYDEBUG == 1
			g_print("    final budget: %d:'%s' : budg[%d]=%.2f\n", entry->key, entry->name, pos, tmp_budget[pos] );
			#endif
		}


		// compute spent for each transaction */
		DB( g_print("\n+ compute spent from transactions\n") );


		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;
		gint pos = 0;
		gdouble trn_amount;

			//DB( g_print("%d, get ope: %s :: acc=%d, cat=%d, mnt=%.2f\n", i, ope->wording, ope->account, ope->category, ope->amount) );

			if( ope->flags & OF_SPLIT )
			{
			guint nbsplit = da_splits_count(ope->splits);
			Split *split;
			
				for(i=0;i<nbsplit;i++)
				{
					split = ope->splits[i];
					switch(tmpfor)
					{
						case BUDG_CATEGORY:
							{
							Category *catentry = da_cat_get(split->kcat);
								if(catentry)
									pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
							}
							break;
						case BUDG_SUBCATEGORY:
							pos = split->kcat;
							break;
					}

					//trn_amount = split->amount;
					trn_amount = hb_amount_base(split->amount, ope->kcur);

					DB( g_print(" -> affecting split %.2f to cat pos %d\n", trn_amount, pos) );

					tmp_spent[pos] += trn_amount;
				
				}
			}
			else
			{
				switch(tmpfor)
				{
					case BUDG_CATEGORY:
						{
						Category *catentry = da_cat_get(ope->kcat);
							if(catentry)
								pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
						}
						break;
					case BUDG_SUBCATEGORY:
						pos = ope->kcat;
						break;
				}

				//trn_amount = ope->amount;
				trn_amount = hb_amount_base(ope->amount, ope->kcur);

				DB( g_print(" -> affecting %.2f to cat pos %d\n", trn_amount, pos) );

				tmp_spent[pos] += trn_amount;

			}

			list = g_list_next(list);
			i++;
		}

		DB( g_print("\nclear and detach model\n") );

		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

		DB( g_print("\n+ insert into treeview\n") );

		/* insert into the treeview */
		for(i=1, id=0; i<=n_result; i++)
		{
		gchar *name, *fullcatname;
		Category *entry;

			fullcatname = NULL;

			entry = da_cat_get(i);
			if( entry == NULL)
				continue;

			if(entry->flags & GF_SUB)
			{
			Category *parent = da_cat_get( entry->parent);

				fullcatname = g_strdup_printf("%s:%s", parent->name, entry->name);
				name = fullcatname;
			}
			else
				name = entry->name;

			if(name == NULL) name  = "(None)";

			//#1553862
			//if( (tmpfor == BUDG_CATEGORY && !(entry->flags & GF_SUB)) || (tmpfor == BUDG_SUBCATEGORY) )
			if( (tmpfor == BUDG_CATEGORY && !(entry->flags & GF_SUB)) || (tmpfor == BUDG_SUBCATEGORY && (entry->flags & GF_SUB)) )
			{
			guint pos;

				pos = 0;
				switch(tmpfor)
				{
					case BUDG_CATEGORY:
						{
						Category *catentry = da_cat_get(i);
							if(catentry)
								pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
						}
						break;
					case BUDG_SUBCATEGORY:
						pos = i;
						break;
				}

				// display expense or income (filter on amount and not category hypothetical flag
				//if( tmpkind !=  (entry->flags & GF_INCOME)) continue;
				if( tmpkind == 1 && tmp_budget[pos] > 0)
					continue;

				if( tmpkind == 2 && tmp_budget[pos] < 0)
					continue;

				DB( g_print(" eval %d '%s' : spen=%.2f bud=%.2f \n", i, name, tmp_spent[pos], tmp_budget[pos] ) );

				if((entry->flags & (GF_BUDGET|GF_FORCED)) || tmp_budget[pos] /*|| tmp_spent[pos]*/)
				{
				gdouble result, rawrate;
				gchar *status;

					result = budget_compute_result(tmp_budget[pos], tmp_spent[pos]);
					rawrate = 0.0;
					if(ABS(tmp_budget[pos]) > 0)
					{
						rawrate = tmp_spent[pos] / tmp_budget[pos];
					}
					else if(tmp_budget[pos] == 0.0)
						rawrate = ABS(tmp_spent[pos]);

					status = "";
					if(rawrate > 1.0)
					{
						status = _(" over");
					}
					else
					{
						if(tmp_budget[pos] < 0.0)
							status = _(" left");
						else if(tmp_budget[pos] > 0.0)
							status = _(" under");
					}

					DB( g_print(" => insert %.2f | %.2f = %.2f '%s'\n", tmp_spent[pos], tmp_budget[pos], result, status ) );

					gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			 		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						LST_BUDGET_POS, id++,
						LST_BUDGET_KEY, pos,
						LST_BUDGET_NAME, name,
						LST_BUDGET_SPENT, tmp_spent[pos],
						LST_BUDGET_BUDGET, tmp_budget[pos],
						LST_BUDGET_RESULT, result,
						LST_BUDGET_STATUS, status,
						-1);

					data->total_spent  += tmp_spent[pos];
					data->total_budget += tmp_budget[pos];
				}
			}

			g_free(fullcatname);

		}

		/* update column 0 title */
		GtkTreeViewColumn *column = gtk_tree_view_get_column( GTK_TREE_VIEW(data->LV_report), 0);
		gtk_tree_view_column_set_title(column, _(CYA_BUDGSELECT[tmpfor]));

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);


		repbudget_update_total(widget, NULL);

		/* update stack chart */
		title = g_strdup_printf(_("Budget for %s"), _(CYA_BUDGSELECT[tmpfor]) );

		ui_chart_stack_set_currency(GTK_CHARTSTACK(data->RE_stack), GLOBALS->kcur);

		/* set chart color scheme */
		ui_chart_stack_set_color_scheme(GTK_CHARTSTACK(data->RE_stack), PREFS->report_color_scheme);
		ui_chart_stack_set_dualdatas(GTK_CHARTSTACK(data->RE_stack), model, _("Budget"), _("Result"), title, NULL);

		g_free(title);
	}

	//DB( g_print(" inserting %i, %f %f\n", i, total_expense, total_income) );

	/* free our memory */
	g_free(tmp_spent);
	g_free(tmp_budget);

}


static void repbudget_update_total(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;

	DB( g_print("\n[repbudget] update total\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	hb_label_set_colvalue(GTK_LABEL(data->TX_total[0]), data->total_spent, GLOBALS->kcur, GLOBALS->minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[1]), data->total_budget, GLOBALS->kcur, GLOBALS->minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[2]), budget_compute_result(data->total_budget, data->total_spent), GLOBALS->kcur, GLOBALS->minor);
	

}


/*
** update sensitivity
*/
static void repbudget_sensitive(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gboolean active;
gboolean sensitive;
gint page;

	DB( g_print("\n[repbudget] sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	active = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	sensitive = page == 0 ? active : FALSE;
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_DETAIL], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail"), sensitive);

}


static void repbudget_toggle(GtkWidget *widget, gpointer user_data)
{
struct repbudget_data *data;
gboolean minor;

	DB( g_print("\n[repbudget] toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repbudget_update_total(widget, NULL);

	//hbfile_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	ui_chart_stack_show_minor(GTK_CHARTSTACK(data->RE_stack), minor);

}

static void repbudget_setup(struct repbudget_data *data)
{
	DB( g_print("\n[repbudget] setup\n") );

	data->txn_queue = g_queue_new ();

	data->filter = da_filter_malloc();
	filter_default_all_set(data->filter);

	data->detail = PREFS->budg_showdetail;
	data->legend = 1;

	/* 3.4 : make int transfer out of stats */
	data->filter->option[FILTER_PAYMODE] = 1;
	data->filter->paymode[PAYMODE_INTXFER] = FALSE;

	filter_preset_daterange_set(data->filter, PREFS->date_range_rep, 0);
	
	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPBUDGET_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPBUDGET_MAXDATE]);

	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);

	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPBUDGET_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPBUDGET_MAXDATE]);

}


static void repbudget_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key;

	DB( g_print("\n[repbudget] selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_BUDGET_KEY, &key, -1);

		DB( g_print(" - active is %d\n", key) );

		repbudget_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	}

	repbudget_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}
/*
**
*/
static gboolean repbudget_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repbudget_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("\n[repbudget] start dispose\n") );

	g_queue_free (data->txn_queue);

	da_filter_free(data->filter);
	
	g_free(data);

	//store position and size
	wg = &PREFS->bud_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);

	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

	DB( g_print("\n[repbudget] end dispose\n") );

	return FALSE;
}


// the window creation
GtkWidget *repbudget_window_new(void)
{
struct repbudget_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table, *entry;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct repbudget_data));
	if(!data) return NULL;

	DB( g_print("\n[repbudget] new\n") );

	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Budget report"));

	//set the window icon
	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_REP_BUDGET);


	//window contents
	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//control part
	table = gtk_grid_new ();
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	//alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	//gtk_container_add(GTK_CONTAINER(alignment), table);
    gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_SMALL);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);

	row = 0;
	label = make_label_group(_("Display"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("_For:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_BUDGSELECT);
	data->CY_for = widget;
	gtk_grid_attach (GTK_GRID (table), data->CY_for, 2, row, 1, 1);


	row++;
	label = make_label_widget(_("_Kind:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_KIND);
	data->CY_kind = widget;
	gtk_grid_attach (GTK_GRID (table), data->CY_kind, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Euro _minor"));
	data->CM_minor = widget;
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
	actions = gtk_action_group_new ("Account");

	//as we use gettext
   	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	// data to action callbacks is set here (data)
	gtk_action_group_add_actions (actions, entries, n_entries, data);

	gtk_action_group_add_toggle_actions (actions,
					   toggle_entries, n_toggle_entries,
					   data);


	/* set which action should have priority in the toolbar */
	action = gtk_action_group_get_action(actions, "List");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Stack");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Detail");
	g_object_set(action, "is_important", TRUE, NULL);
	g_object_set(action, "active", PREFS->budg_showdetail, NULL);

	action = gtk_action_group_get_action(actions, "Refresh");
	g_object_set(action, "is_important", TRUE, NULL);


 	ui = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	gtk_window_add_accel_group (GTK_WINDOW (window), gtk_ui_manager_get_accel_group (ui));

	if (!gtk_ui_manager_add_ui_from_string (ui, ui_info, -1, &error))
	{
		g_message ("building UI failed: %s", error->message);
		g_error_free (error);
	}

	data->ui = ui;

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), SPACING_SMALL);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	widget = make_label(NULL, 0.5, 0.5);
	gimp_label_set_attributes (GTK_LABEL (widget), PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL, -1);
	data->TX_daterange = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);


	entry = gtk_label_new(NULL);
	data->TX_total[2] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Result:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[1] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Budget:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[0] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Spent:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);


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
	treeview = create_list_budget();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = widget;
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);

    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//page: 2d bar
	//widget = gtk_chart_new(CHART_TYPE_COL);
	widget = ui_chart_stack_new();
	data->RE_stack = widget;
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);


	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail))), "minor", (gpointer)data->CM_minor);

	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repbudget_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repbudget_toggle), NULL);


	data->handler_id[HID_REPBUDGET_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repbudget_range_change), NULL);

    g_signal_connect (data->CY_for , "changed", G_CALLBACK (repbudget_compute), (gpointer)data);
    g_signal_connect (data->CY_kind, "changed", G_CALLBACK (repbudget_compute), (gpointer)data);

    data->handler_id[HID_REPBUDGET_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repbudget_date_change), (gpointer)data);
    data->handler_id[HID_REPBUDGET_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repbudget_date_change), (gpointer)data);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (repbudget_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_detail), "row-activated", G_CALLBACK (repbudget_detail_onRowActivated), NULL);


	//setup, init and show window
	repbudget_setup(data);


	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);


	//setup, init and show window
	wg = &PREFS->bud_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);



	gtk_widget_show_all (window);


	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	//check for any account included into the budget or warn
	{
	guint count =0;
	GList *lacc, *list;

		lacc = list = g_hash_table_get_values(GLOBALS->h_acc);

		while (list != NULL)
		{
		Account *acc;
			acc = list->data;
			if((acc->flags & (AF_CLOSED|AF_NOREPORT))) goto next1;
			if(!(acc->flags & AF_NOBUDGET))
				count++;
		next1:
			list = g_list_next(list);
		}
		g_list_free(lacc);

		if(count <= 0)
		{
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_WARNING,
				_("No account is defined to be part of the budget."),
				_("You should include some accounts from the account dialog.")
				);
		}

	
	
	}



	//gtk_widget_hide(data->GR_detail);

	repbudget_sensitive(window, NULL);
	repbudget_update_detail(window, NULL);

	if( PREFS->date_range_rep != 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_rep);
	else
		repbudget_compute(window, NULL);

	return(window);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
static gint budget_listview_compare_funct (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint retval = 0;
//Category *entry1, *entry2;
gchar *entry1, *entry2;

	gtk_tree_model_get(model, a, LST_BUDGET_NAME, &entry1, -1);
	gtk_tree_model_get(model, b, LST_BUDGET_NAME, &entry2, -1);

	retval = hb_string_utf8_compare(entry1, entry2);
	//retval = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
	//if(!retval)
	//{
	//	retval = hb_string_utf8_compare(entry1->name, entry2->name);
	//}

    return retval;
}


static void budget_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
   {
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gint column_id = GPOINTER_TO_INT(user_data);

	gtk_tree_model_get(model, iter, column_id, &value, -1);

	if( value )
	{
		hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, GLOBALS->kcur, GLOBALS->minor);

		if( column_id == LST_BUDGET_RESULT)
			color = get_minimum_color_amount (value, 0.0);
		else
			color = get_normal_color_amount(value);

		g_object_set(renderer,
			"foreground",  color,
			"text", buf,
			NULL);
	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}
}



static void budget_result_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
   {
gdouble  value;
gchar *color;
gchar *status;
gint column_id = GPOINTER_TO_INT(user_data);

	gtk_tree_model_get(model, iter, 
		column_id, &value,
		LST_BUDGET_STATUS, &status,
		-1);

	if( value )
	{
		color = get_minimum_color_amount (value, 0.0);
		g_object_set(renderer,
			"foreground",  color,
			"text", status,
			NULL);
	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}
}



static GtkTreeViewColumn *amount_list_budget_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, budget_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

/*
** create our statistic list
*/
static GtkWidget *create_list_budget(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	DB( g_print("\n[repbudget] create list\n") );


	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_BUDGET,
		G_TYPE_INT,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,     //spent
		G_TYPE_DOUBLE,     //budget
		G_TYPE_DOUBLE,     //result
		G_TYPE_STRING	   //status
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);

	/* column: Name */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, ope_result_cell_data_function, NULL, NULL);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_BUDGET_NAME);
	//gtk_tree_view_column_set_sort_column_id (column, LST_STAT_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Expense */
	column = amount_list_budget_column(_("Spent"), LST_BUDGET_SPENT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = amount_list_budget_column(_("Budget"), LST_BUDGET_BUDGET);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Result */
	column = amount_list_budget_column(_("Result"), LST_BUDGET_RESULT);
	renderer = gtk_cell_renderer_text_new ();
	//g_object_set(renderer, "xalign", 0.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, budget_result_cell_data_function, GINT_TO_POINTER(LST_BUDGET_RESULT), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* sort */
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), budget_listview_compare_funct, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

/*
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_POS   , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_POS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_SPENT , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_SPENT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_BUDGET, stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_BUDGET), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_BUDGET_RESULT , stat_list_compare_func, GINT_TO_POINTER(LST_BUDGET_RESULT), NULL);
*/

	return(view);
}

