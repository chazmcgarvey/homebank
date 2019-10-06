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

#include "rep-time.h"

#include "list-operation.h"
#include "gtk-chart.h"
#include "gtk-dateentry.h"

#include "dsp-mainwindow.h"
#include "ui-account.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-filter.h"
#include "ui-transaction.h"
#include "ui-tag.h"


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
static void ui_reptime_action_viewcolumn(GtkAction *action, gpointer user_data);
static void ui_reptime_action_viewline(GtkAction *action, gpointer user_data);
static void ui_reptime_action_detail(GtkAction *action, gpointer user_data);
//static void ui_reptime_action_filter(GtkAction *action, gpointer user_data);
static void ui_reptime_action_refresh(GtkAction *action, gpointer user_data);

static void ui_reptime_list_set_cur(GtkTreeView *treeview, guint32 kcur);


static GtkRadioActionEntry radio_entries[] = {
  { "List"    , ICONNAME_HB_VIEW_LIST  , N_("List")   , NULL,    N_("View results as list")  , 0 },
  { "Line"    , ICONNAME_HB_VIEW_LINE  , N_("Line")   , NULL,    N_("View results as lines") , 1 },
  { "Column"  , ICONNAME_HB_VIEW_COLUMN, N_("Column") , NULL,    N_("View results as column"), 2 },
};
static guint n_radio_entries = G_N_ELEMENTS (radio_entries);

static GtkActionEntry entries[] = {
//  { "Filter"  , ICONNAME_HB_FILTER    , N_("Filter") , NULL,   N_("Edit the filter"), G_CALLBACK (ui_reptime_action_filter) },
  { "Refresh" , ICONNAME_HB_REFRESH   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (ui_reptime_action_refresh) },

//  { "Export" , ICONNAME_HB_FILE_EXPORT, N_("Export")  , NULL,   N_("Export as CSV"), G_CALLBACK (ui_reptime_action_export) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static GtkToggleActionEntry toggle_entries[] = {
  { "Detail", ICONNAME_HB_OPE_SHOW,                    /* name, icon-name */
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
"    <toolitem action='Column'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"      <separator/>"
//"    <toolitem action='Filter'/>"
"    <toolitem action='Refresh'/>"
"      <separator/>"
//"    <toolitem action='Export'/>"
//		replaced by a menubutton
"  </toolbar>"
"</ui>";


static void ui_reptime_date_change(GtkWidget *widget, gpointer user_data);
static void ui_reptime_range_change(GtkWidget *widget, gpointer user_data);
static void ui_reptime_detail(GtkWidget *widget, gpointer user_data);
static void ui_reptime_update(GtkWidget *widget, gpointer user_data);
static void ui_reptime_compute(GtkWidget *widget, gpointer user_data);
static void ui_reptime_sensitive(GtkWidget *widget, gpointer user_data);
static void ui_reptime_toggle_detail(GtkWidget *widget, gpointer user_data);
static void ui_reptime_toggle_minor(GtkWidget *widget, gpointer user_data);
static void ui_reptime_update_daterange(GtkWidget *widget, gpointer user_data);
static GtkWidget *ui_list_reptime_create(void);

static gint ui_list_reptime_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata);
static GString *ui_list_reptime_to_string(GtkTreeView *treeview, gboolean clipboard);


HbKvData CYA_REPORT_SRC_TREND[] = { 
	{ REPORT_SRC_ACCOUNT, 	N_("Account") }, 
	{ REPORT_SRC_CATEGORY,	N_("Category") }, 
	{ REPORT_SRC_PAYEE,		N_("Payee") },
	{ REPORT_SRC_TAG,		N_("Tag") },
	{ 0, NULL }
};


HbKvData CYA_REPORT_INTVL[] = { 
	{ REPORT_INTVL_DAY,		N_("Day") }, 
	{ REPORT_INTVL_WEEK,	N_("Week") },
	{ REPORT_INTVL_MONTH,	N_("Month") }, 
	{ REPORT_INTVL_QUARTER,	N_("Quarter") },
	{ REPORT_INTVL_HALFYEAR,N_("Half Year") }, 
	{ REPORT_INTVL_YEAR,	N_("Year") }, 
	{ 0, NULL }
};


extern gchar *RA_REPORT_TIME_MODE[];
extern gchar *CYA_ABMONTHS[];



/* action functions -------------------- */


static void ui_reptime_action_viewlist(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	data->charttype = CHART_TYPE_NONE;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	ui_reptime_sensitive(data->window, NULL);
}

static void ui_reptime_action_viewline(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	data->charttype = CHART_TYPE_LINE;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	ui_reptime_sensitive(data->window, NULL);
	ui_reptime_update(data->window, NULL);
}


static void ui_reptime_action_viewcolumn(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	data->charttype = CHART_TYPE_COL;
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	ui_reptime_sensitive(data->window, NULL);
	ui_reptime_update(data->window, NULL);

}


static void ui_reptime_action_mode (GtkRadioAction *action, GtkRadioAction *current, gpointer user_data)
{
gint value;

	value = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(action));
	switch( value )
	{
		case 0:
			ui_reptime_action_viewlist(GTK_ACTION(action), user_data);
			break;
		case 1:
			ui_reptime_action_viewline(GTK_ACTION(action), user_data);
			break;
		case 2:
			ui_reptime_action_viewcolumn (GTK_ACTION(action), user_data);
			break;
	}
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

/*static void ui_reptime_action_export(GtkAction *action, gpointer user_data)
{
struct ui_reptime_data *data = user_data;

	ui_reptime_export_csv(data->window, NULL);
}*/



/* ======================== */



/*
** ============================================================================
*/

/*
** return the year list position correponding to the passed date
*/

static void ui_reptime_date_change(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	DB( g_print("\n[reptime] date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_date_entry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_date_entry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPTIME_RANGE]);
	hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_range), FLT_RANGE_OTHER);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPTIME_RANGE]);

	ui_reptime_compute(widget, NULL);
	ui_reptime_update_daterange(widget, NULL);

}


static void ui_reptime_update_quickdate(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	DB( g_print("\n[reptime] update quickdate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);
	
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
	
	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);

}


static void ui_reptime_range_change(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint range;

	DB( g_print("\n[reptime] range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_range));

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range, data->accnum);

		ui_reptime_update_quickdate(widget, NULL);
		
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




static void ui_reptime_update(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkTreeModel *model;
gint page;
gint tmpsrc;
gchar *title;

	DB( g_print("\n[reptime] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	//byamount = 0;
	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	//tmpintvl = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_intvl));

	// ensure not exp & inc for piechart
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	DB( g_print(" page %d\n\n", page) );
	//DB( g_print(" tmpintvl %d\n\n", tmpintvl) );

	//column = LST_REPTIME_POS;
	//DB( g_print(" sort on column %d\n\n", column) );
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), column, GTK_SORT_DESCENDING);

	gtk_chart_show_legend(GTK_CHART(data->RE_line), FALSE, FALSE);
	gtk_chart_show_xval(GTK_CHART(data->RE_line), TRUE);

	//visible = (tmpmode == REPORT_RESULT_TOTAL) ? TRUE : FALSE;
	//gtk_chart_show_average(GTK_CHART(data->RE_line), data->average, visible);

	//TRANSLATORS: example 'Category Over Time'
	title = g_strdup_printf(_("%s Over Time"), hbtk_get_label(CYA_REPORT_SRC_TREND, tmpsrc) );
	gtk_chart_set_datas(GTK_CHART(data->RE_line), model, LST_REPTIME_AMOUNT, title, NULL);
	g_free(title);
	
	if(page == 1)
	{
		DB(	g_print(" change chart type to %d\n", data->charttype) );
		gtk_chart_set_type (GTK_CHART(data->RE_line), data->charttype);
		gtk_chart_set_showmono(GTK_CHART(data->RE_line), TRUE);
	}
	
}


static void ui_reptime_export_result_clipboard(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkClipboard *clipboard;
GString *node;

	DB( g_print("\n[reptime] export result clipboard\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	node = ui_list_reptime_to_string(GTK_TREE_VIEW(data->LV_report), TRUE);

	clipboard = gtk_clipboard_get_default(gdk_display_get_default());
	gtk_clipboard_set_text(clipboard, node->str, node->len);

	g_string_free(node, TRUE);
}


static void ui_reptime_export_result_csv(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gchar *filename = NULL;
GString *node;
GIOChannel *io;
gchar *name;
gint tmpsrc;

	DB( g_print("\n[reptime] export result csv\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	name = g_strdup_printf("hb-reptime_%s.csv", hbtk_get_label(CYA_REPORT_SRC_TREND, tmpsrc) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );
		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			node = ui_list_reptime_to_string(GTK_TREE_VIEW(data->LV_report), FALSE);
			g_io_channel_write_chars(io, node->str, -1, NULL, NULL);
			g_io_channel_unref (io);
			g_string_free(node, TRUE);
		}
		g_free( filename );
	}
	g_free(name);
}


static void ui_reptime_export_detail_clipboard(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkClipboard *clipboard;
GString *node;

	DB( g_print("\n[reptime] export detail clipboard\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	node = list_txn_to_string(GTK_TREE_VIEW(data->LV_detail), TRUE);

	clipboard = gtk_clipboard_get_default(gdk_display_get_default());
	gtk_clipboard_set_text(clipboard, node->str, node->len);

	g_string_free(node, TRUE);
}


static void ui_reptime_export_detail_csv(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gchar *filename = NULL;
GString *node;
GIOChannel *io;
gchar *name;
gint tmpsrc;

	DB( g_print("\n[reptime] export detail csv\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	name = g_strdup_printf("hb-reptime-detail_%s.csv", hbtk_get_label(CYA_REPORT_SRC_TREND, tmpsrc) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			node = list_txn_to_string(GTK_TREE_VIEW(data->LV_detail), FALSE);
			g_io_channel_write_chars(io, node->str, -1, NULL, NULL);

			g_io_channel_unref (io);
			g_string_free(node, TRUE);
		}

		g_free( filename );
	}

	g_free(name);
}


static void ui_reptime_update_for(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint tmpsrc;
gboolean visible;

	DB( g_print("\n[reptime] update for\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));

	visible = tmpsrc == REPORT_SRC_ACCOUNT ? TRUE : FALSE;
	hb_widget_visible(data->LB_acc, visible);
	hb_widget_visible(data->PO_acc, visible);

	visible = tmpsrc == REPORT_SRC_CATEGORY ? TRUE : FALSE;
	hb_widget_visible(data->LB_cat, visible);
	hb_widget_visible(data->PO_cat, visible);
	
	visible = tmpsrc == REPORT_SRC_PAYEE ? TRUE : FALSE;
	hb_widget_visible(data->LB_pay, visible);
	hb_widget_visible(data->PO_pay, visible);

	visible = tmpsrc == REPORT_SRC_TAG ? TRUE : FALSE;
	hb_widget_visible(data->LB_tag, visible);
	hb_widget_visible(data->PO_tag, visible);
	
}



static void ui_reptime_for(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;

	DB( g_print("\n[reptime] for\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	ui_reptime_update_for(widget, data);

	ui_reptime_compute(widget, data);
}


//TODO: this is temporary, as at end we will enable filter 
static void ui_reptime_compute_set_filter(Filter *flt, gint tmpsrc, guint32 selkey)
{
	//inactive all filters
	flt->option[FILTER_CATEGORY] = 0;
	flt->option[FILTER_PAYEE] = 0;
	flt->option[FILTER_ACCOUNT] = 0;
	flt->option[FILTER_TEXT] = 0;

	switch(tmpsrc)
	{
		case REPORT_SRC_ACCOUNT:
			flt->option[FILTER_ACCOUNT] = 1;
			filter_status_acc_clear_except(flt, selkey);
			break;
		case REPORT_SRC_CATEGORY:
			flt->option[FILTER_CATEGORY] = 1;
			filter_status_cat_clear_except(flt, selkey);
			break;
		case REPORT_SRC_PAYEE:
			flt->option[FILTER_PAYEE] = 1;
			filter_status_pay_clear_except(flt, selkey);
			break;
		case REPORT_SRC_TAG:
			flt->option[FILTER_TEXT] = 1;
			filter_set_tag_by_id(flt, selkey);
			break;
	}
	

}


static void ui_reptime_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
guint active = GPOINTER_TO_INT(user_data);
guint tmpintvl;
guint32 from;
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[reptime] detail\n") );

	//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
	tmpintvl = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_intvl));

	//ui_reptime_compute_set_filter was already called here
	
	//get our min max date
	from = data->filter->mindate;
	//to   = data->filter->maxdate;

	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if(data->detail)
	{
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		/* fill in the model */
		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;
		guint pos;

			if(filter_txn_match(data->filter, ope) == 1)
			{
				pos = report_interval_get_pos(tmpintvl, from, ope);
				if( pos == active )
				{
				gdouble dtlamt = report_txn_amount_get(data->filter, ope);

					gtk_list_store_insert_with_values (GTK_LIST_STORE(model), &iter, -1,
						MODEL_TXN_POINTER, ope,
					    MODEL_TXN_SPLITAMT, dtlamt,
						-1);
				}
			}

			list = g_list_next(list);
		}

		/* Re-attach model to view */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), model);
		g_object_unref(model);

		gtk_tree_view_columns_autosize( GTK_TREE_VIEW(data->LV_detail) );
	}

}


static void ui_reptime_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
gint tmpsrc, tmpintvl, range, showempty;
guint32 from, to;
gboolean cumul;
gboolean showall;

gdouble cumulation, average;

GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint id;
guint n_result, i;
gdouble *tmp_amount;
guint32 selkey;

	DB( g_print("\n[reptime] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	tmpsrc    = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	tmpintvl  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_intvl));
	cumul     = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cumul));
	showall   = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_all));
	range     = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_range));
	showempty = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_showempty));

	selkey = 0;
	
	data->accnum = 0;
	switch(tmpsrc)
	{
		case REPORT_SRC_ACCOUNT:
			selkey = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			if(showall == FALSE) 
				data->accnum = selkey;
			break;
		case REPORT_SRC_CATEGORY:
			selkey = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_cat));
			break;
		case REPORT_SRC_PAYEE:
			selkey = ui_pay_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_pay));
			break;
		case REPORT_SRC_TAG:
			selkey = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->PO_tag));
			break;
	}

	if( showall == TRUE )
	{
		data->filter->option[FILTER_CATEGORY] = 0;
		data->filter->option[FILTER_PAYEE] = 0;
		data->filter->option[FILTER_ACCOUNT] = 0;
		data->filter->option[FILTER_TEXT] = 0;
	}
	else	
		ui_reptime_compute_set_filter(data->filter, tmpsrc, selkey);

	
	DB( g_print(" source=%d-%s, intvl=%d-%s:: key=%d\n", tmpsrc, hbtk_get_label(CYA_REPORT_SRC_TREND, tmpsrc), tmpintvl, hbtk_get_label(CYA_REPORT_INTVL, tmpintvl), selkey) );

	//to remove > 5.0.2
	//#1715532 5.0.5: no... but only showall
	if( (showall == TRUE) && (range == FLT_RANGE_ALLDATE) )
	{
		filter_preset_daterange_set(data->filter, data->filter->range, data->accnum);
		ui_reptime_update_quickdate(widget, NULL);
	}

	//get our min max date
	from = data->filter->mindate;
	to   = data->filter->maxdate;
	if(to < from) return;

	g_queue_free (data->txn_queue);
	data->txn_queue = hbfile_transaction_get_partial(data->filter->mindate, data->filter->maxdate);

	n_result = report_interval_count(tmpintvl, from, to);

	DB( g_print(" %s :: n_result=%d\n", hbtk_get_label(CYA_REPORT_SRC_TREND, tmpsrc), n_result) );

	/* allocate some memory */
	tmp_amount = g_malloc0((n_result+2) * sizeof(gdouble));

	if(tmp_amount)
	{
		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;

			//DB( g_print("\n** testing '%s', cat=%d==> %d\n", ope->memo, ope->kcat, filter_txn_match(data->filter, ope)) );

			if( (filter_txn_match(data->filter, ope) == 1) )
			{
			guint pos;
			gdouble trn_amount;

				trn_amount = report_txn_amount_get(data->filter, ope);

				 //#1829603 Multi currencies problem in Trend Time Report 
				if( ! ( tmpsrc == REPORT_SRC_ACCOUNT && showall == FALSE) )
					trn_amount = hb_amount_base(trn_amount, ope->kcur);
				
				pos = report_interval_get_pos(tmpintvl, from, ope);
				if( pos <= n_result )
				{
					DB( g_print("** pos=%d : add of %.2f\n", pos, trn_amount) );
					tmp_amount[pos] += trn_amount;
				}
				else
				{
					DB( g_print("** pos=%d : invalid offset\n", pos) );
				}
			}
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
		gchar intvlname[64];
		gdouble value;

			if( !showempty && tmp_amount[i] == 0 )
				continue;

			report_interval_snprint_name(intvlname, sizeof(intvlname)-1, tmpintvl, from, i);

			DB( g_print("try to insert item %d\n", i) );

			cumulation += tmp_amount[i];
			value = (cumul == TRUE) ? cumulation : tmp_amount[i];

			DB( g_print(" inserting %2d, '%s', %9.2f\n", i, intvlname, value) );

	    	gtk_list_store_insert_with_values (GTK_LIST_STORE(model), &iter, -1, 
				LST_REPTIME_POS, id++,
				LST_REPTIME_KEY, i,
				LST_REPTIME_TITLE, intvlname,
				LST_REPTIME_AMOUNT, value,
				-1);

		}

		// set chart and listview currency
		guint32 kcur = GLOBALS->kcur;
		if( (showall == FALSE) && (tmpsrc == REPORT_SRC_ACCOUNT) )
		{
		Account *acc = da_acc_get(selkey);
			if( acc != NULL )
				kcur = acc->kcur;

			gtk_chart_set_overdrawn(GTK_CHART(data->RE_line), acc->minimum);
		}

		ui_reptime_list_set_cur(GTK_TREE_VIEW(data->LV_report), kcur);
		gtk_chart_set_currency(GTK_CHART(data->RE_line), kcur);

		
		/* update column 0 title */
		GtkTreeViewColumn *column = gtk_tree_view_get_column( GTK_TREE_VIEW(data->LV_report), 0);
		if(column)
			gtk_tree_view_column_set_title(column, hbtk_get_label(CYA_REPORT_INTVL, tmpintvl) );

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);
		
		//update average
		gtk_label_set_text(GTK_LABEL(data->TX_info), "");

		if( cumul == TRUE )
		{
		gchar *info;
		gchar   buf[128];

			average = cumulation / n_result;
			data->average = average;

			hb_strfmon(buf, 127, average, kcur, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor)) );

			info = g_strdup_printf(_("Average: %s"), buf);
			gtk_label_set_text(GTK_LABEL(data->TX_info), info);
			g_free(info);
		}
	}

	/* free our memory */
	g_free(tmp_amount);
	
	ui_reptime_update(widget, user_data);
}


static void ui_reptime_sensitive(GtkWidget *widget, gpointer user_data)
{
struct ui_reptime_data *data;
GtkAction *action;
gboolean visible, sensitive;
gint page;

	DB( g_print("\n[reptime] sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	visible = page == 0 ? TRUE : FALSE;
	action = gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail");
	gtk_action_set_visible (action, visible);
	//action = gtk_ui_manager_get_action(data->ui, "/ToolBar/Export");
	//gtk_action_set_visible (action, visible);
	//sensitive = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);
	//gtk_action_set_sensitive(action, sensitive);
	hb_widget_visible (data->BT_export, visible);

	visible = page == 0 ? FALSE : TRUE;
	hb_widget_visible(data->LB_zoomx, visible);
	hb_widget_visible(data->RG_zoomx, visible);

	sensitive = gtk_tree_model_iter_n_children(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail)), NULL) > 0 ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->MI_detailtoclip, sensitive);
	gtk_widget_set_sensitive(data->MI_detailtocsv, sensitive);

}


static void ui_reptime_detail_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct ui_reptime_data *data;
Transaction *active_txn;
gboolean result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[reptime] A detail row has been double-clicked!\n") );

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
			ui_reptime_compute(data->window, NULL);
		}

		da_transaction_free (old_txn);
	}
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

	DB( g_print("(stats) toggledetail to %d\n", data->detail) );

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
	gtk_widget_set_sensitive(GTK_WIDGET(data->PO_tag), showall^1);

	ui_reptime_compute(widget, data);

}



/*
**
*/
static void ui_reptime_setup(struct ui_reptime_data *data, guint32 accnum)
{
	DB( g_print("\n[reptime] setup\n") );

	data->txn_queue = g_queue_new ();

	data->filter = da_flt_malloc();
	filter_reset(data->filter);

	data->detail = 0;

	/* 3.4 : make int transfer out of stats */
	//todo: for compatibility with < 5.3, keep this unset, but normally it should be set
	//data->filter->option[FILTER_PAYMODE] = 1;
	//data->filter->paymode[PAYMODE_INTXFER] = FALSE;

	filter_preset_daterange_set(data->filter, PREFS->date_range_rep, data->accnum);
	
	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);

	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);

	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPTIME_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPTIME_MAXDATE]);


	DB( g_print(" populate\n") );
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_REPORT);
	if( accnum )
		ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_acc), accnum);
	else
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_acc), 0);

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->h_pay);
	ui_pay_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_pay), 0);

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->h_cat);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_cat), 0);

	ui_tag_combobox_populate(GTK_COMBO_BOX_TEXT(data->PO_tag));
	
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

	g_queue_free (data->txn_queue);

	da_flt_free(data->filter);

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
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview;
GtkWidget *label, *widget, *table;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
//GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct ui_reptime_data));
	if(!data) return NULL;

	DB( g_print("\n[reptime] new\n") );


	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", window, data) );


	gtk_window_set_title (GTK_WINDOW (window), _("Trend Time Report"));

	//set the window icon
	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_REP_TIME);


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
	label = make_label_widget(_("_View by:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	//widget = make_cycle(label, CYA_TIMESELECT);
	widget = hbtk_combo_box_new_with_data(label, CYA_REPORT_SRC_TREND);
	data->CY_src = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("A_ccount:"));
	data->LB_acc = label;
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Category:"));
	data->LB_cat = label;
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = ui_cat_comboboxentry_new(label);
	data->PO_cat = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Payee:"));
	data->LB_pay = label;
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = ui_pay_comboboxentry_new(label);
	data->PO_pay = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Tag:"));
	data->LB_tag = label;
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = ui_tag_combobox_new(label);
	data->PO_tag = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Select _all"));
	data->CM_all = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Cumulate"));
	data->CM_cumul = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Inter_val:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	//widget = make_cycle(label, CYA_REPORT_INTVL);
	widget = hbtk_combo_box_new_with_data(label, CYA_REPORT_INTVL);
	data->CY_intvl = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Show empty line"));
	data->CM_showempty = widget;
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
	data->CY_range = make_daterange(label, DATE_RANGE_CUSTOM_DISABLE);
	gtk_grid_attach (GTK_GRID (table), data->CY_range, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_From:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	data->PO_mindate = gtk_date_entry_new(label);
	gtk_grid_attach (GTK_GRID (table), data->PO_mindate, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_To:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	data->PO_maxdate = gtk_date_entry_new(label);
	gtk_grid_attach (GTK_GRID (table), data->PO_maxdate, 2, row, 1, 1);


	//part: info + report
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	//ui manager
	actions = gtk_action_group_new ("default");

	//as we use gettext
   	gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);

	// data to action callbacks is set here (data)
	gtk_action_group_add_radio_actions (actions, radio_entries, n_radio_entries, 0, G_CALLBACK(ui_reptime_action_mode), data);

	gtk_action_group_add_actions (actions, entries, n_entries, data);

	gtk_action_group_add_toggle_actions (actions,
					   toggle_entries, n_toggle_entries,
					   data);


	/* set which action should have priority in the toolbar */
	//action = gtk_action_group_get_action(actions, "List");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Line");
	//g_object_set(action, "is_important", TRUE, NULL);

	/*action = gtk_action_group_get_action(actions, "Column");
	g_object_set(action, "is_important", TRUE, NULL);*/

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

	//add export menu button
	GtkToolItem *toolitem;
	GtkWidget *menu, *menuitem, *image;

	menu = gtk_menu_new ();
	//gtk_widget_set_halign (menu, GTK_ALIGN_END);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Result to clipboard"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_reptime_export_result_clipboard), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Result to CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_reptime_export_result_csv), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Detail to clipboard"));
	data->MI_detailtoclip = menuitem;
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_reptime_export_detail_clipboard), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Detail to CSV"));
	data->MI_detailtocsv = menuitem;
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_reptime_export_detail_csv), data);

	gtk_widget_show_all (menu);

	widget = gtk_menu_button_new();
	data->BT_export = widget;
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(widget)), GTK_STYLE_CLASS_FLAT);

	//gtk_menu_button_set_direction (GTK_MENU_BUTTON(widget), GTK_ARROW_DOWN);
	//gtk_widget_set_halign (widget, GTK_ALIGN_END);
	image = gtk_image_new_from_icon_name (ICONNAME_HB_FILE_EXPORT, GTK_ICON_SIZE_LARGE_TOOLBAR);
	g_object_set (widget, "image", image, "popup", GTK_MENU(menu),  NULL);

	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), widget);
	gtk_toolbar_insert(GTK_TOOLBAR(data->TB_bar), GTK_TOOL_ITEM(toolitem), -1);

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
	treeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
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

	//duplicate, see below
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_intvl), 1);

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

	g_signal_connect (data->CY_src, "changed", G_CALLBACK (ui_reptime_for), (gpointer)data);
	data->handler_id[HID_REPTIME_VIEW] = g_signal_connect (data->CY_intvl, "changed", G_CALLBACK (ui_reptime_compute), (gpointer)data);

	g_signal_connect (data->CM_showempty, "toggled", G_CALLBACK (ui_reptime_compute), NULL);

	//setup, init and show window
	ui_reptime_setup(data, accnum);

	g_signal_connect (data->CM_all, "toggled", G_CALLBACK (ui_reptime_toggle_showall), NULL);
	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (ui_reptime_compute), NULL);
	g_signal_connect (data->PO_cat, "changed", G_CALLBACK (ui_reptime_compute), NULL);
	g_signal_connect (data->PO_pay, "changed", G_CALLBACK (ui_reptime_compute), NULL);
	g_signal_connect (data->PO_tag, "changed", G_CALLBACK (ui_reptime_compute), NULL);

	g_signal_connect (data->RG_zoomx, "value-changed", G_CALLBACK (ui_reptime_zoomx_callback), NULL);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (ui_reptime_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_detail), "row-activated", G_CALLBACK (ui_reptime_detail_onRowActivated), NULL);


	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

	//setup, init and show window
	wg = &PREFS->tme_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	//todo: !! here
	hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_intvl), REPORT_INTVL_MONTH);

	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	ui_reptime_update_for(window, data);
	//gtk_widget_hide(data->GR_detail);



	ui_reptime_sensitive(window, NULL);
	ui_reptime_update_detail(window, NULL);

	DB( g_print("range: %d\n", PREFS->date_range_rep) );

	if( PREFS->date_range_rep != 0)
		hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_range), PREFS->date_range_rep);
	else
		ui_reptime_compute(window, NULL);

	return window;
}


/*
** ============================================================================
*/


static GString *ui_list_reptime_to_string(GtkTreeView *treeview, gboolean clipboard)
{
GString *node;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
const gchar *format;

	node = g_string_new(NULL);

	// header
	format = (clipboard == TRUE) ? "%s\t%s\n" : "%s;%s\n";
	g_string_append_printf(node, format, _("Time slice"), _("Amount"));

	model = gtk_tree_view_get_model(treeview);
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

		format = (clipboard == TRUE) ? "%s\t%.2f\n" : "%s;%.2f\n";
		g_string_append_printf(node, format, name, amount);

		//leak
		g_free(name);
		
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	//DB( g_print("text is:\n%s", node->str) );

	return node;
}


static void ui_reptime_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
guint32 kcur = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(gtk_tree_view_column_get_tree_view(col)), "kcur_data"));

	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &value, -1);

	if( value )
	{
		hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur, GLOBALS->minor);

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


static void ui_reptime_list_set_cur(GtkTreeView *treeview, guint32 kcur)
{
	g_object_set_data(G_OBJECT(treeview), "kcur_data", GUINT_TO_POINTER(kcur));
}


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

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);

	/* column: Name */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Time slice"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
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



