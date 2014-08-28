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


#include "homebank.h"

#include "rep_time.h"

#include "list_operation.h"
#include "gtk-chart.h"
#include "gtk-dateentry.h"

#include "dsp_mainwindow.h"
#include "ui-account.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-filter.h"

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
static void ui_reptime_action_viewlist(GtkAction *action, gpointer user_data);
static void ui_reptime_action_viewline(GtkAction *action, gpointer user_data);
static void ui_reptime_action_detail(GtkAction *action, gpointer user_data);
//static void ui_reptime_action_filter(GtkAction *action, gpointer user_data);
static void ui_reptime_action_refresh(GtkAction *action, gpointer user_data);
static void ui_reptime_action_export(GtkAction *action, gpointer user_data);

//static void ui_reptime_list_set_cur(GtkTreeView *treeview, guint32 kcur);


static GtkActionEntry entries[] = {
  { "List"    , "hb-view-list" , N_("List")   , NULL,    N_("View results as list"), G_CALLBACK (ui_reptime_action_viewlist) },
  { "Line"    , "hb-view-line" , N_("Line")   , NULL,   N_("View results as lines"), G_CALLBACK (ui_reptime_action_viewline) },

//  { "Filter"  , "hb-filter"    , N_("Filter") , NULL,   N_("Edit the filter"), G_CALLBACK (ui_reptime_action_filter) },
  { "Refresh" , GTK_STOCK_REFRESH   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (ui_reptime_action_refresh) },

  { "Export" , "hb-file-export", N_("Export")  , NULL,   N_("Export as CSV"), G_CALLBACK (ui_reptime_action_export) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static GtkToggleActionEntry toggle_entries[] = {
  { "Detail", "hb-ope-show",                    /* name, stock id */
     N_("Detail"), NULL,                    /* label, accelerator */
    N_("Toggle detail"),                                    /* tooltip */
    G_CALLBACK (ui_reptime_action_detail),
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
//"    <toolitem action='Filter'/>"
"    <toolitem action='Refresh'/>"
"      <separator/>"
"    <toolitem action='Export'/>"
"  </toolbar>"
"</ui>";


static void ui_reptime_date_change(GtkWidget *widget, gpointer user_data);
static void ui_reptime_range_change(GtkWidget *widget, gpointer user_data);
static void ui_reptime_detail(GtkWidget *widget, gpointer user_data);
static void ui_reptime_update(GtkWidget *widget, gpointer user_data);
static void ui_reptime_export_csv(GtkWidget *widget, gpointer user_data);
static void ui_reptime_compute(GtkWidget *widget, gpointer user_data);
static void ui_reptime_sensitive(GtkWidget *widget, gpointer user_data);
static void ui_reptime_toggle_detail(GtkWidget *widget, gpointer user_data);
static void ui_reptime_toggle_minor(GtkWidget *widget, gpointer user_data);
static void ui_reptime_update_daterange(GtkWidget *widget, gpointer user_data);
static GtkWidget *ui_list_reptime_create(void);

static gint ui_list_reptime_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata);



gchar *CYA_TIMESELECT[] = { N_("Account"), N_("Category"), N_("Payee"), NULL };



gchar *CYA_VIEWBY[] = { N_("Day"), N_("Week"), N_("Month"), N_("Quarter"), N_("Year"), NULL };

//extern gchar *CYA_FLT_SELECT[];

gchar *CYA_ABMONTHS[] =
{
NULL,
N_("Jan"),
N_("Feb"),
N_("Mar"),
N_("Apr"),
N_("May"),
N_("Jun"),
N_("Jul"),
N_("Aug"),
N_("Sep"),
N_("Oct"),
N_("Nov"),
N_("Dec"),
};

/* action functions -------------------- */

static void ui_reptime_action_viewlist(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	ui_reptime_sensitive(data->window, NULL);
}










static void ui_reptime_action_viewline(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	ui_reptime_sensitive(data->window, NULL);

}


static void ui_reptime_action_detail(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	ui_reptime_toggle_detail(data->window, NULL);
}

/*
static void ui_reptime_action_filter(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	//debug
	//create_deffilter_window(data->filter, TRUE);

	if(create_deffilter_window(data->filter, TRUE) != GTK_RESPONSE_REJECT)
		ui_reptime_compute(data->window, NULL);
}
*/

static void ui_reptime_action_refresh(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	ui_reptime_compute(data->window, NULL);
}

static void ui_reptime_action_export(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	ui_reptime_export_csv(data->window, NULL);
}



/* ======================== */



/*
** ============================================================================
*/




/*
** return the month list position correponding to the passed date
*/
static gint DateInMonth(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
gint pos;

	//debug
	// this return sometimes -1, -2 which is wrong

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	pos = ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1);

	//g_print(" from=%d-%d ope=%d-%d => %d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos);

	g_date_free(date2);
	g_date_free(date1);

	return(pos);
}

static gint DateInQuarter(guint32 from, guint32 opedate)
{
GDate *date1, *date2;
gint pos;

	//debug
	// this return sometimes -1, -2 which is wrong

	date1 = g_date_new_julian(from);
	date2 = g_date_new_julian(opedate);

	pos = (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/3;

	DB( g_print(" from=%d-%d ope=%d-%d => %d\n", g_date_get_month(date1), g_date_get_year(date1), g_date_get_month(date2), g_date_get_year(date2), pos) );

	g_date_free(date2);
	g_date_free(date1);

	return(pos);
}
/*
** return the year list position correponding to the passed date
*/
static gint DateInYear(guint32 from, guint32 opedate)
{
GDate *date;
gint year_from, year_ope, pos;

	date = g_date_new_julian(from);
	year_from = g_date_get_year(date);
	g_date_set_julian(date, opedate);
	year_ope = g_date_get_year(date);
	g_date_free(date);

	pos = year_ope - year_from;

	//g_print(" from=%d ope=%d => %d\n", year_from, year_ope, pos);

	return(pos);
}

static void ui_reptime_date_change(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	DB( g_print("\n[reptime] date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_dateentry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_dateentry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPTIME_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), FLT_RANGE_OTHER);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPTIME_RANGE]);

	ui_reptime_compute(widget, NULL);
	ui_reptime_update_daterange(widget, NULL);

}



static void ui_reptime_range_change(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint range;

	DB( g_print("\n[reptime] range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);
		
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);
		
		ui_reptime_compute(widget, NULL);
		ui_reptime_update_daterange(widget, NULL);
	}
	
}

static void ui_reptime_update_daterange(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gchar *daterange;

	DB( g_print("\n[reptime] update daterange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	daterange = filter_daterange_text_get(data->filter);
	gtk_label_set_markup(GTK_LABEL(data->TX_daterange), daterange);
	g_free(daterange);
}


static void ui_reptime_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
guint active = GPOINTER_TO_INT(user_data);
guint tmpfor, tmpslice;
gboolean showall;
guint32 from, to;
guint i;
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;
guint32 selkey;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[reptime] detail\n") );

	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpslice = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));
	showall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_all));
	selkey = 0;

	switch(tmpfor)
	{
		case FOR_REPTIME_ACCOUNT:
			selkey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			break;
		case FOR_REPTIME_CATEGORY:
			selkey = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_cat));
			break;
		case FOR_REPTIME_PAYEE:
			selkey = ui_pay_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_pay));
			break;
	}

	//DB( g_print(" for=%d, view by=%d :: key=%d\n", tmpfor, tmpslice, selkey) );

	//get our min max date
	from = data->filter->mindate;
	to   = data->filter->maxdate;

	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if(data->detail)
	{
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		/* fill in the model */
		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Transaction *ope = list->data;
		Account *acc;

			//DB( g_print(" get %s\n", ope->ope_Word) );

			acc = da_acc_get(ope->kacc);
			if(acc == NULL) goto next1;
			if((acc->flags & (AF_CLOSED|AF_NOREPORT))) goto next1;

			//filter here
			if( !(ope->flags & OF_REMIND) && ope->date >= from && ope->date <= to)
			{
			guint32 pos = 0;
			gboolean include = FALSE;

				switch(tmpfor)
				{
					case FOR_REPTIME_ACCOUNT:
						if( selkey == ope->kacc )
							include = TRUE;
						break;
					case FOR_REPTIME_CATEGORY:
					{
					Category *catentry;
					
						if( ope->flags & OF_SPLIT )
						{
						guint nbsplit = da_transaction_splits_count(ope);
						Split *split;
						
							for(i=0;i<nbsplit;i++)
							{
								split = ope->splits[i];
								catentry = da_cat_get(split->kcat);
								if(catentry != NULL)	//#1340142
								{
									if( selkey == catentry->parent || selkey == catentry->key )
										include = TRUE;

								}
							}
						}
						else
						{							
							catentry = da_cat_get(ope->kcat);
							if(catentry != NULL)	//#1340142
							{
								if( selkey == catentry->parent || selkey == catentry->key )
									include = TRUE;

							}
						}
					}
						break;
					case FOR_REPTIME_PAYEE:
						if( selkey == ope->kpay )
							include = TRUE;
						break;
				}

				if( include == TRUE || showall == TRUE )
				{

					switch(tmpslice)
					{
						case GROUPBY_REPTIME_DAY:
							pos = ope->date - from;
							break;

						case GROUPBY_REPTIME_WEEK:
							pos = (ope->date - from)/7;
							break;

						case GROUPBY_REPTIME_MONTH:
							pos = DateInMonth(from, ope->date);
							break;

						case GROUPBY_REPTIME_QUARTER:
							pos = DateInQuarter(from, ope->date);
							break;

						case GROUPBY_REPTIME_YEAR:
							pos = DateInYear(from, ope->date);
							break;
					}

					DB( g_print("** pos=%d\n", pos) );

					//insert
					if( pos == active )
					{

						gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				 		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							LST_DSPOPE_DATAS, ope,
							-1);
					}
				}


			}
next1:
			list = g_list_next(list);
		}

		/* Re-attach model to view */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), model);
		g_object_unref(model);

	}

}


static void ui_reptime_update(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkTreeModel *model;
gint tmpfor;
gchar *title;
//gboolean xval;

	DB( g_print("\n[reptime] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	//byamount = 0;
	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	//tmpslice = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

	// ensure not exp & inc for piechart
	//page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	//DB( g_print(" page %d\n\n", page) );
	//DB( g_print(" tmpslice %d\n\n", tmpslice) );


	//column = LST_REPTIME_POS;
	//DB( g_print(" sort on column %d\n\n", column) );
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), column, GTK_SORT_DESCENDING);

	//gtk_chart_set_datas(GTK_CHART(data->RE_line), model, LST_STAT_AMOUNT, NULL);
	gtk_chart_show_legend(GTK_CHART(data->RE_line), FALSE, FALSE);
	gtk_chart_show_xval(GTK_CHART(data->RE_line), TRUE);

	////TRANSLATORS: example 'Expense by Category'
	title = g_strdup_printf(_("%s Over Time"), _(CYA_TIMESELECT[tmpfor]) );
	gtk_chart_set_datas(GTK_CHART(data->RE_line), model, LST_REPTIME_AMOUNT, title);
	g_free(title);

}

static void ui_reptime_export_csv(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gchar *filename = NULL;
GIOChannel *io;
gchar *outstr, *name;
gint tmpfor;

	DB( g_print("\n[reptime] export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));

	name = g_strdup_printf("hb-ui_reptime_%s.csv", CYA_TIMESELECT[tmpfor]);

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			// header
			outstr = g_strdup_printf("%s;%s;\n", _("Time slice"), _("Amount"));
			g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

			while (valid)
			{
			gchar *name;
			gdouble amount;

				gtk_tree_model_get (model, &iter,
					//LST_REPTIME_KEY, i,
					LST_REPTIME_TITLE  , &name,
					LST_REPTIME_AMOUNT , &amount,

					-1);

				outstr = g_strdup_printf("%s;%.2f\n", name, amount);
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


static void ui_reptime_for(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint page;

	DB( g_print("\n[reptime] for\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	page  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_select), page);

	ui_reptime_compute(widget, data);
}


static void ui_reptime_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint tmpfor, tmpslice;
guint32 from, to;
gboolean cumul;
gboolean showall;

gdouble cumulation;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint id;
guint n_result, i;
GDate *date1, *date2;
gdouble *tmp_amount;
guint32 selkey;

	DB( g_print("\n[reptime] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpfor  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_for));
	tmpslice = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));
	cumul = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cumul));
	showall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_all));
	selkey = 0;

	switch(tmpfor)
	{
		case FOR_REPTIME_ACCOUNT:
			selkey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			break;
		case FOR_REPTIME_CATEGORY:
			selkey = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_cat));
			break;
		case FOR_REPTIME_PAYEE:
			selkey = ui_pay_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_pay));
			break;
	}

	DB( g_print(" for=%d, view by=%d :: key=%d\n", tmpfor, tmpslice, selkey) );

	/* do nothing if no transaction */
	if(g_list_length(GLOBALS->ope_list) == 0) return;

	//get our min max date
	from = data->filter->mindate;
	to   = data->filter->maxdate;
	if(to < from) return;

	/* count number or results */
	switch(tmpslice)
	{
		case GROUPBY_REPTIME_DAY:
			n_result = 1 + (to - from);
			break;
		case GROUPBY_REPTIME_WEEK:
			n_result = 1 + ((to - from) / 7);
			break;
		case GROUPBY_REPTIME_MONTH:
			date1 = g_date_new_julian(from);
			date2 = g_date_new_julian(to);
			n_result = 1 + ((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1);
			g_date_free(date2);
			g_date_free(date1);
			break;
		case GROUPBY_REPTIME_QUARTER:
			date1 = g_date_new_julian(from);
			date2 = g_date_new_julian(to);
			n_result = 1 + (((g_date_get_year(date2) - g_date_get_year(date1)) * 12) + g_date_get_month(date2) - g_date_get_month(date1))/3;
			g_date_free(date2);
			g_date_free(date1);
			break;
		case GROUPBY_REPTIME_YEAR:
			date1 = g_date_new_julian(from);
			date2 = g_date_new_julian(to);
			n_result = 1 + g_date_get_year(date2) - g_date_get_year(date1);
			g_date_free(date2);
			g_date_free(date1);
			break;
		default:
			n_result = 0;
	}

	DB( g_print(" %s :: n_result=%d\n", CYA_TIMESELECT[tmpfor], n_result) );

	/* allocate some memory */

	tmp_amount = g_malloc0((n_result+2) * sizeof(gdouble));

	if(tmp_amount)
	{
	Account *acc;	
		
		/* set currency */
		/*
		if( (tmpfor == FOR_REPTIME_ACCOUNT) && (showall == FALSE) )
		{
			if(acc = da_acc_get(selkey))
			{
				//ui_reptime_list_set_cur(GTK_TREE_VIEW(data->LV_report), acc->kcur);
				//gtk_chart_set_currency(GTK_CHART(data->RE_line), acc->kcur);
			}
		}
		else
		{
			//ui_reptime_list_set_cur(GTK_TREE_VIEW(data->LV_report), GLOBALS->kcur);
			//gtk_chart_set_currency(GTK_CHART(data->RE_line), GLOBALS->kcur);
		}*/
			
	/* compute the results */
		list = g_list_first(GLOBALS->ope_list);
		while (list != NULL)
		{
		Transaction *ope = list->data;

			//debug
			DB( g_print("** testing '%s', cat=%d==> %d\n", ope->wording, ope->kcat, filter_test(data->filter, ope)) );

			acc = da_acc_get(ope->kacc);
			if(acc == NULL) goto next1;
			if((acc->flags & (AF_CLOSED|AF_NOREPORT))) goto next1;


			// add usage of payee or category
			if( !(ope->flags & OF_REMIND) && ope->date >= from && ope->date <= to)
			//if( (filter_test(data->filter, ope) == 1) )
			{
			gboolean include = FALSE;

				switch(tmpfor)
				{
					case FOR_REPTIME_ACCOUNT:
						if( selkey == ope->kacc )
							include = TRUE;
						break;
					case FOR_REPTIME_CATEGORY:
					{
					Category *catentry;
					
						if( ope->flags & OF_SPLIT )
						{
						guint nbsplit = da_transaction_splits_count(ope);
						Split *split;
						
							for(i=0;i<nbsplit;i++)
							{
								split = ope->splits[i];
								catentry = da_cat_get(split->kcat);
								if(catentry != NULL)	//#1340142
								{
									if( selkey == catentry->parent || selkey == catentry->key )
										include = TRUE;
								}
							}
						}
						else
						{							
							catentry = da_cat_get(ope->kcat);
							if(catentry != NULL)	//#1340142
							{
								if( selkey == catentry->parent || selkey == catentry->key )
									include = TRUE;
							}
						}
						
					}
						break;
					case FOR_REPTIME_PAYEE:
						if( selkey == ope->kpay )
							include = TRUE;
						break;
				}

				if( include == TRUE || showall == TRUE)
				{
				guint32 pos = 0;
				gdouble trn_amount;
				
					switch(tmpslice)
					{
						case GROUPBY_REPTIME_DAY:
							pos = ope->date - from;
							break;

						case GROUPBY_REPTIME_WEEK:
							pos = (ope->date - from)/7;
							break;

						case GROUPBY_REPTIME_MONTH:
							pos = DateInMonth(from, ope->date);
							break;

						case GROUPBY_REPTIME_QUARTER:
							pos = DateInQuarter(from, ope->date);
							break;

						case GROUPBY_REPTIME_YEAR:
							pos = DateInYear(from, ope->date);
							break;
					}

					acc = da_acc_get(ope->kacc);
					trn_amount = 0.0;
					
					if( tmpfor == FOR_REPTIME_CATEGORY && ope->flags & OF_SPLIT )
					{
					guint nbsplit = da_transaction_splits_count(ope);
					Split *split;
					Category *catentry;
					
						for(i=0;i<nbsplit;i++)
						{
							split = ope->splits[i];
							catentry = da_cat_get(split->kcat);
							if(catentry != NULL)	//#1340142
							{
								if( selkey == catentry->parent || selkey == catentry->key )
									trn_amount += split->amount;
							}
						}
					}
					else
						trn_amount = ope->amount;

					
					//trn_amount = to_base_amount(trn_amount, acc->kcur);

					DB( g_print("** pos=%d will add %.2f to \n", pos, trn_amount) );

					tmp_amount[pos] += trn_amount;

				}
			}
next1:
			list = g_list_next(list);
		}

		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

		cumulation = 0.0;

		/* insert into the treeview */
		for(i=0, id=0; i<n_result; i++)
		{
		gchar *name, *fullcatname;
		gdouble value;
		gchar buffer[64];
		GDate *date;

			name = NULL;
			fullcatname = NULL;


			DB( g_print("try to insert item %d\n", i) );

			/* get the result name */
			switch(tmpslice)
			{
				case GROUPBY_REPTIME_DAY:
					date = g_date_new_julian (from + i);
					g_date_strftime (buffer, 63, PREFS->date_format, date);
					g_date_free(date);
					name = buffer;
					break;

				case GROUPBY_REPTIME_WEEK:
					date = g_date_new_julian(from);
					g_date_add_days(date, i*7);
					//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
					g_snprintf(buffer, 63, "%d-%d", g_date_get_year(date), g_date_get_monday_week_of_year(date));
					g_date_free(date);
					name = buffer;
					break;

				case GROUPBY_REPTIME_MONTH:
					date = g_date_new_julian(from);
					g_date_add_months(date, i);
					//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
					g_snprintf(buffer, 63, "%d-%s", g_date_get_year(date), _(CYA_ABMONTHS[g_date_get_month(date)]));
					g_date_free(date);
					name = buffer;
					break;

				case GROUPBY_REPTIME_QUARTER:
					date = g_date_new_julian(from);
					g_date_add_months(date, i*3);
					//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
					g_snprintf(buffer, 63, "%d-%d", g_date_get_year(date), ((g_date_get_month(date)-1)/3)+1);
					g_date_free(date);
					name = buffer;
					break;

				case GROUPBY_REPTIME_YEAR:
					date = g_date_new_julian(from);
					g_date_add_years(date, i);
					g_snprintf(buffer, 63, "%d", g_date_get_year(date));
					g_date_free(date);
					name = buffer;
					break;
			}

			cumulation += tmp_amount[i];
			value = cumul == TRUE ? cumulation : tmp_amount[i];


			//DB( g_print(" inserting %2d, '%s', %9.2f\n", i, name, value) );

	    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_REPTIME_POS, id++,
				LST_REPTIME_KEY, i,
				LST_REPTIME_TITLE, name,
				LST_REPTIME_AMOUNT, value,
				-1);

			g_free(fullcatname);
		}

		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);
	}

	/* free our memory */
	g_free(tmp_amount);


	ui_reptime_update(widget, user_data);

}





/*
** update sensitivity
*/
static void ui_reptime_sensitive(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gboolean active;
gboolean sensitive;
gint page;

	DB( g_print("\n[reptime] sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	active = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	sensitive = page == 0 ? active : FALSE;
//	gtk_widget_set_sensitive(data->TB_buttons[ACTION_REPBUDGET_DETAIL], sensitive);
	gtk_action_set_sensitive(gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail"), sensitive);

	//view = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_view));

}

static void ui_reptime_update_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

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
			gtk_tree_model_get(model, &iter, LST_REPTIME_KEY, &key, -1);

			DB( g_print(" - active is %d\n", key) );

			ui_reptime_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
		}



		gtk_widget_show(data->GR_detail);
	}
	else
		gtk_widget_hide(data->GR_detail);
}




static void ui_reptime_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->detail ^= 1;

	DB( printf("(stats) toggledetail to %d\n", data->detail) );

	ui_reptime_update_detail(widget, user_data);

}

static void ui_reptime_zoomx_callback(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gdouble value;

	DB( g_print("\n[reptime] zoomx\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_range_get_value(GTK_RANGE(data->RG_zoomx));

	DB( g_print(" + scale is %.2f\n", value) );

	gtk_chart_set_barw(GTK_CHART(data->RE_line), value);

}



static void ui_reptime_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	DB( g_print("\n[reptime] toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	//hbfile_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	gtk_chart_show_minor(GTK_CHART(data->RE_line), GLOBALS->minor);

}


static void ui_reptime_toggle_showall(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gboolean showall;

	DB( g_print("\n[reptime] toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	showall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_all));

	gtk_widget_set_sensitive(GTK_WIDGET(data->PO_acc), showall^1);
	gtk_widget_set_sensitive(GTK_WIDGET(data->PO_cat), showall^1);
	gtk_widget_set_sensitive(GTK_WIDGET(data->PO_pay), showall^1);

	ui_reptime_compute(widget, data);

}



/*
**
*/
static void ui_reptime_setup(struct ui_reptime_data *data, guint32 accnum)
{
	DB( g_print("\n[reptime] setup\n") );

	data->detail = 0;


	data->filter = da_filter_malloc();
	filter_default_all_set(data->filter);

	/* 3.4 : make int transfer out of stats */
	data->filter->option[FILTER_PAYMODE] = 1;
	data->filter->paymode[PAYMODE_INTXFER] = FALSE;

	filter_preset_daterange_set(data->filter, PREFS->date_range_rep);
	
	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);

	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);

	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);


	DB( g_print(" populate\n") );
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_REPORT);
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_acc), accnum);

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->h_pay);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_pay), 0);

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->h_cat);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_cat), 0);

	DB( g_print(" all ok\n") );

}



static void ui_reptime_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key = -1;

	DB( g_print("\n[reptime] selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_REPTIME_KEY, &key, -1);

	}

	DB( g_print(" - active is %d\n", key) );

	ui_reptime_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	ui_reptime_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


/*
**
*/
static gboolean ui_reptime_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct ui_reptime_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("\n[reptime] dispose\n") );

	da_filter_free(data->filter);

	g_free(data);

	//store position and size
	wg = &PREFS->tme_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);

	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );



	//enable define windows
	GLOBALS->define_off--;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

	return FALSE;
}

// the window creation
GtkWidget *ui_reptime_window_new(guint32 accnum)
{
struct ui_reptime_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *hbox2, *notebook, *treeview;
GtkWidget *label, *widget, *table, *alignment;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct ui_reptime_data));
	if(!data) return NULL;

	DB( g_print("\n[reptime] new\n") );


	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Trend Time Report"));

	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "report_stats.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_REP_TIME);


	//window contents
	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), mainvbox);

	hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start (GTK_BOX (mainvbox), hbox, TRUE, TRUE, 0);

	//control part
	table = gtk_table_new (6, 3, FALSE);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.0, 0.0, 0.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
    gtk_box_pack_start (GTK_BOX (hbox), alignment, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), HB_BOX_SPACING);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(_("Display"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_For:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_TIMESELECT);
	data->CY_for = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_for, 2, 3, row, row+1);

	row++;
	notebook = gtk_notebook_new();
	data->GR_select = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
	gtk_table_attach_defaults (GTK_TABLE (table), notebook, 1, 3, row, row+1);

	//account
	hbox2 = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox2, NULL);
	label = gtk_label_new_with_mnemonic (_("_Account:"));
	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_box_pack_start (GTK_BOX (hbox2), widget, TRUE, TRUE, 0);

	//category
	hbox2 = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox2, NULL);
	label = gtk_label_new_with_mnemonic (_("_Category:"));
	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	widget = ui_cat_comboboxentry_new(label);
	data->PO_cat = widget;
	gtk_box_pack_start (GTK_BOX (hbox2), widget, TRUE, TRUE, 0);

	//payee
	hbox2 = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox2, NULL);
	label = gtk_label_new_with_mnemonic (_("_Payee:"));
	gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);
	widget = ui_pay_comboboxentry_new(label);
	data->PO_pay = widget;
	gtk_box_pack_start (GTK_BOX (hbox2), widget, TRUE, TRUE, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Select _all"));
	data->CM_all = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Cumulate"));
	data->CM_cumul = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 3, row, row+1);

	row++;
	label = make_label(_("_View by:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_VIEWBY);
	data->CY_view = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Minor currency"));
	data->CM_minor = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 3, row, row+1);

	row++;
	label = make_label(_("_Zoom X:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_scale(label);
	data->RG_zoomx = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 2, 3, row, row+1);


	row++;
	widget = gtk_hseparator_new();
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 3, row, row+1);

	row++;
	label = make_label(_("Date filter"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Range:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->CY_range = make_daterange(label, FALSE);
	gtk_table_attach_defaults (GTK_TABLE (table), data->CY_range, 2, 3, row, row+1);

	row++;
	label = make_label(_("_From:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->PO_mindate = gtk_dateentry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), data->PO_mindate, 2, 3, row, row+1);

	row++;
	label = make_label(_("_To:"), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data->PO_maxdate = gtk_dateentry_new();
	gtk_table_attach_defaults (GTK_TABLE (table), data->PO_maxdate, 2, 3, row, row+1);


	//part: info + report
	vbox = gtk_vbox_new (FALSE, 0);
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

	action = gtk_action_group_get_action(actions, "Line");
	g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Detail");
	g_object_set(action, "is_important", TRUE, NULL);

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
	data->actions = actions;

	//toolbar
	data->TB_bar = gtk_ui_manager_get_widget (ui, "/ToolBar");
	gtk_box_pack_start (GTK_BOX (vbox), data->TB_bar, FALSE, FALSE, 0);

	//infos
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), HB_BOX_SPACING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);


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
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, NULL);

	widget = gtk_scrolled_window_new (NULL, NULL);
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (widget), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = ui_list_reptime_create();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = widget;
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (widget), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = create_list_transaction(TRN_LIST_TYPE_DETAIL, PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);

    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//page: lines
	widget = gtk_chart_new(CHART_TYPE_LINE);
	data->RE_line = widget;
	//gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.suffix_symbol);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor), GLOBALS->minor);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_view), 1);

	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail))), "minor", (gpointer)data->CM_minor);



		/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (ui_reptime_dispose), (gpointer)data);

	g_signal_connect (data->CM_cumul, "toggled", G_CALLBACK (ui_reptime_compute), NULL);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (ui_reptime_toggle_minor), NULL);

    data->handler_id[HID_REPTIME_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (ui_reptime_date_change), (gpointer)data);
    data->handler_id[HID_REPTIME_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (ui_reptime_date_change), (gpointer)data);

	data->handler_id[HID_REPTIME_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (ui_reptime_range_change), NULL);

	g_signal_connect (data->CY_for, "changed", G_CALLBACK (ui_reptime_for), (gpointer)data);
	data->handler_id[HID_REPTIME_VIEW] = g_signal_connect (data->CY_view, "changed", G_CALLBACK (ui_reptime_compute), (gpointer)data);

	//setup, init and show window
	ui_reptime_setup(data, accnum);

	g_signal_connect (data->CM_all, "toggled", G_CALLBACK (ui_reptime_toggle_showall), NULL);
	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (ui_reptime_compute), NULL);
	g_signal_connect (data->PO_cat, "changed", G_CALLBACK (ui_reptime_compute), NULL);
	g_signal_connect (data->PO_pay, "changed", G_CALLBACK (ui_reptime_compute), NULL);

	g_signal_connect (data->RG_zoomx, "value-changed", G_CALLBACK (ui_reptime_zoomx_callback), NULL);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (ui_reptime_selection), NULL);



	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

	//setup, init and show window
	wg = &PREFS->tme_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_view), GROUPBY_REPTIME_MONTH);

	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	//gtk_widget_hide(data->GR_detail);



	ui_reptime_sensitive(window, NULL);
	ui_reptime_update_detail(window, NULL);

	DB( g_print("range: %d\n", PREFS->date_range_rep) );

	if( PREFS->date_range_rep != 0)
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_rep);
	else
		ui_reptime_compute(window, NULL);

	return window;
}

/*
** ============================================================================
*/


static void ui_reptime_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
//guint32 kcur = (guint32)g_object_get_data(G_OBJECT(gtk_tree_view_column_get_tree_view(col)), "kcur_data");

	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &value, -1);

	if( value )
	{
		mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, GLOBALS->minor);
		//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur);

		color = get_normal_color_amount(value);

		g_object_set(renderer,
			"foreground",  color,
			"text", buf,
			NULL);	}
	else
	{
		g_object_set(renderer, "text", "", NULL);
	}
}


static GtkTreeViewColumn *amount_list_ui_reptime_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_reptime_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

/*
static void ui_reptime_list_set_cur(GtkTreeView *treeview, guint32 kcur)
{
	g_object_set_data(G_OBJECT(treeview), "kcur_data", (guint32)kcur);
}*/



/*
** create our statistic list
*/
static GtkWidget *ui_list_reptime_create(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_REPTIME,
		G_TYPE_INT,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), PREFS->rules_hint);

	/* column: Name */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Time slice"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, ope_result_cell_data_function, NULL, NULL);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_REPTIME_TITLE);
	//gtk_tree_view_column_set_sort_column_id (column, LST_REPTIME_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = amount_list_ui_reptime_column(_("Amount"), LST_REPTIME_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPTIME_POS    , ui_list_reptime_compare_func, GINT_TO_POINTER(LST_REPTIME_POS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPTIME_AMOUNT, ui_list_reptime_compare_func, GINT_TO_POINTER(LST_REPTIME_AMOUNT), NULL);

	return(view);
}

static gint ui_list_reptime_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint sortcol = GPOINTER_TO_INT(userdata);
gint retval = 0;
gint pos1, pos2;
gdouble val1, val2;

	gtk_tree_model_get(model, a,
		LST_REPTIME_POS, &pos1,
		sortcol, &val1,
		-1);
	gtk_tree_model_get(model, b,
		LST_REPTIME_POS, &pos2,
		sortcol, &val2,
		-1);
/*
	if(pos1 == -1) return(1);
	if(pos2 == -1) return(-1);
*/

	if(sortcol == LST_REPTIME_POS)
		retval = pos2 - pos1;
	else
		retval = (ABS(val1) - ABS(val2)) > 0 ? 1 : -1;

	//DB( g_print(" sort %d=%d or %.2f=%.2f :: %d\n", pos1,pos2, val1, val2, ret) );

    return retval;
  }

