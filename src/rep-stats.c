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

#include "rep-stats.h"

#include "list-operation.h"
#include "gtk-chart.h"
#include "gtk-dateentry.h"

#include "dsp-mainwindow.h"
#include "ui-account.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-filter.h"
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
static void ui_repdist_action_viewlist(GtkAction *action, gpointer user_data);
static void ui_repdist_action_viewbar(GtkAction *action, gpointer user_data);
static void ui_repdist_action_viewpie(GtkAction *action, gpointer user_data);
static void ui_repdist_action_detail(GtkAction *action, gpointer user_data);
static void ui_repdist_action_legend(GtkAction *action, gpointer user_data);
static void ui_repdist_action_rate(GtkAction *action, gpointer user_data);
static void ui_repdist_action_filter(GtkAction *action, gpointer user_data);
static void ui_repdist_action_refresh(GtkAction *action, gpointer user_data);


static GtkRadioActionEntry radio_entries[] = {
  { "List"    , ICONNAME_HB_VIEW_LIST   , N_("List")   , NULL,    N_("View results as list"),   0 },
  { "Column"  , ICONNAME_HB_VIEW_COLUMN , N_("Column") , NULL,    N_("View results as column"), 1 },
  { "Donut"   , ICONNAME_HB_VIEW_DONUT  , N_("Donut")  , NULL,    N_("View results as donut"),  2 },
};
static guint n_radio_entries = G_N_ELEMENTS (radio_entries);


static GtkActionEntry entries[] = {
  { "Filter"  , ICONNAME_HB_FILTER      , N_("Filter") , NULL,   N_("Edit filter"), G_CALLBACK (ui_repdist_action_filter) },
  { "Refresh" , ICONNAME_HB_REFRESH        , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (ui_repdist_action_refresh) },

  //{ "Export"  , ICONNAME_HB_FILE_EXPORT , N_("Export")  , NULL,   N_("Export as CSV"), G_CALLBACK (ui_repdist_action_export) },
};
static guint n_entries = G_N_ELEMENTS (entries);


static GtkToggleActionEntry toggle_entries[] = {
  { "Detail", ICONNAME_HB_OPE_SHOW,                    /* name, icon-name */
     N_("Detail"), NULL,                    /* label, accelerator */
    N_("Toggle detail"),                                    /* tooltip */
    G_CALLBACK (ui_repdist_action_detail),
    FALSE },                                    /* is_active */

  { "Legend", ICONNAME_HB_SHOW_LEGEND,                    /* name, icon-name */
     N_("Legend"), NULL,                    /* label, accelerator */
    N_("Toggle legend"),                                    /* tooltip */
    G_CALLBACK (ui_repdist_action_legend),
    TRUE },                                    /* is_active */

  { "Rate", ICONNAME_HB_SHOW_RATE,                    /* name, icon-name */
     N_("Rate"), NULL,                    /* label, accelerator */
    N_("Toggle rate"),                                    /* tooltip */
    G_CALLBACK (ui_repdist_action_rate),
    FALSE },                                    /* is_active */

};
static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);



static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='List'/>"
"    <toolitem action='Column'/>"
"    <toolitem action='Donut'/>"
"      <separator/>"
"    <toolitem action='Detail'/>"
"    <toolitem action='Legend'/>"
"    <toolitem action='Rate'/>"
"      <separator/>"
"    <toolitem action='Filter'/>"
"    <toolitem action='Refresh'/>"
"      <separator/>"
//"    <toolitem action='Export'/>"
//		replaced by a menubutton
"  </toolbar>"
"</ui>";



static void ui_repdist_date_change(GtkWidget *widget, gpointer user_data);
static void ui_repdist_range_change(GtkWidget *widget, gpointer user_data);
static void ui_repdist_detail(GtkWidget *widget, gpointer user_data);
static void ui_repdist_update(GtkWidget *widget, gpointer user_data);
static void ui_repdist_update_total(GtkWidget *widget, gpointer user_data);
static void ui_repdist_compute(GtkWidget *widget, gpointer user_data);
static void ui_repdist_sensitive(GtkWidget *widget, gpointer user_data);
static void ui_repdist_toggle_detail(GtkWidget *widget, gpointer user_data);
static void ui_repdist_toggle_legend(GtkWidget *widget, gpointer user_data);
static void ui_repdist_toggle_minor(GtkWidget *widget, gpointer user_data);
static void ui_repdist_toggle_rate(GtkWidget *widget, gpointer user_data);
static GtkWidget *ui_list_repdist_create(void);
static void ui_repdist_update_daterange(GtkWidget *widget, gpointer user_data);
static void ui_repdist_update_date_widget(GtkWidget *widget, gpointer user_data);

static GString *ui_list_repdist_to_string(GtkTreeView *treeview, gboolean clipboard);
static gint ui_list_repdist_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata);



HbKvData CYA_REPORT_SRC[] = {
	{ REPORT_SRC_CATEGORY,		N_("Category") },
	{ REPORT_SRC_SUBCATEGORY,	N_("Subcategory") },
	{ REPORT_SRC_PAYEE,			N_("Payee") },
	{ REPORT_SRC_ACCOUNT,		N_("Account") },
	{ REPORT_SRC_TAG,			N_("Tag") },
	{ REPORT_SRC_MONTH,			N_("Month") },
	{ REPORT_SRC_YEAR,			N_("Year") },
	{ 0, NULL }
};


HbKvData CYA_REPORT_TYPE[] = { 
	{ REPORT_TYPE_ALL,		N_("Exp. & Inc.") },
	{ REPORT_TYPE_EXPENSE,	N_("Expense") },
	{ REPORT_TYPE_INCOME,	N_("Income") },
	{ REPORT_TYPE_BALANCE,	N_("Balance")} ,
	{ 0, NULL }
};

extern gchar *CYA_REPORT_MODE[];
extern HbKvData CYA_REPORT_INTVL[];

extern gchar *CYA_ABMONTHS[];

/* action functions -------------------- */

static void ui_repdist_action_viewlist(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 0);
	ui_repdist_sensitive(data->window, NULL);
}

static void ui_repdist_action_viewbar(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);
	gtk_chart_set_type (GTK_CHART(data->RE_chart), CHART_TYPE_COL);
	ui_repdist_sensitive(data->window, NULL);
}


static void ui_repdist_action_viewpie(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;
gint tmpview;

	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->GR_result), 1);

	gtk_chart_set_type (GTK_CHART(data->RE_chart), CHART_TYPE_PIE);
	ui_repdist_sensitive(data->window, NULL);

	tmpview = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_type));

	// ensure not exp & inc for piechart
	if( tmpview == REPORT_TYPE_ALL )
	{
		//g_signal_handler_block(data->CY_type, data->handler_id[HID_REPDIST_VIEW]);
		hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_type), REPORT_TYPE_EXPENSE);
		//g_signal_handler_unblock(data->CY_type, data->handler_id[HID_REPDIST_VIEW]);
	}

}


static void ui_repdist_action_mode (GtkRadioAction *action, GtkRadioAction *current, gpointer user_data)
{
gint value;

	value = gtk_radio_action_get_current_value(GTK_RADIO_ACTION(action));
	switch( value )
	{
		case 0:
			ui_repdist_action_viewlist(GTK_ACTION(action), user_data);
			break;
		case 1:
			ui_repdist_action_viewbar(GTK_ACTION(action), user_data);
			break;
		case 2:
			ui_repdist_action_viewpie(GTK_ACTION(action), user_data);
			break;
	}
}


static void ui_repdist_action_detail(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	ui_repdist_toggle_detail(data->window, NULL);
}

static void ui_repdist_action_legend(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	ui_repdist_toggle_legend(data->window, NULL);
}

static void ui_repdist_action_rate(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	ui_repdist_toggle_rate(data->window, NULL);
}

static void ui_repdist_action_filter(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	//debug
	//create_deffilter_window(data->filter, TRUE);

	if(ui_flt_manage_dialog_new(GTK_WINDOW(data->window), data->filter, TRUE, FALSE) != GTK_RESPONSE_REJECT)
	{
		ui_repdist_compute(data->window, NULL);
		ui_repdist_update_date_widget(data->window, NULL);
		ui_repdist_update_daterange(data->window, NULL);

		g_signal_handler_block(data->CY_range, data->handler_id[HID_REPDIST_RANGE]);
		hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_range), FLT_RANGE_OTHER);
		g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPDIST_RANGE]);

	}
}

static void ui_repdist_action_refresh(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	ui_repdist_compute(data->window, NULL);
}

/*static void ui_repdist_action_export(GtkAction *action, gpointer user_data)
{
struct ui_repdist_data *data = user_data;

	ui_repdist_export_csv(data->window, NULL);
}*/



/* ======================== */



/*
** ============================================================================
*/


static void ui_repdist_date_change(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;

	DB( g_print("\n[repdist] date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_date_entry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_date_entry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPDIST_RANGE]);
	hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_range), FLT_RANGE_OTHER);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPDIST_RANGE]);

	ui_repdist_compute(widget, NULL);
	ui_repdist_update_daterange(widget, NULL);

}


static void ui_repdist_range_change(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gint range;

	DB( g_print("\n[repdist] range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_range));

	if(range != FLT_RANGE_OTHER)
	{
		filter_preset_daterange_set(data->filter, range, 0);

		ui_repdist_update_date_widget(data->window, NULL);

		ui_repdist_compute(data->window, NULL);
		ui_repdist_update_daterange(data->window, NULL);
	}
	else
	{
		if(ui_flt_manage_dialog_new(GTK_WINDOW(data->window), data->filter, TRUE, FALSE) != GTK_RESPONSE_REJECT)
		{
			ui_repdist_update_date_widget(data->window, NULL);
			ui_repdist_compute(data->window, NULL);
			ui_repdist_update_daterange(data->window, NULL);
		}
	}
}


static void ui_repdist_update(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gboolean byamount;
GtkTreeModel		 *model;
gint page, tmpsrc, tmptype, column;
gboolean xval;
gchar *title;

	DB( g_print("\n[repdist] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	byamount = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_byamount));
	tmptype = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_type));
	//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	
	//debug option
	DB( g_print(" option: byamount=%d tmptype=%d '%s' tmpsrc=%d '%s'\n\n", byamount, tmptype, hbtk_get_label(CYA_REPORT_TYPE,tmptype), tmpsrc, hbtk_get_label(CYA_REPORT_SRC,tmpsrc)) );

	// ensure not exp & inc for piechart
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	if( page == 2 && tmptype == REPORT_TYPE_ALL )
	{
		g_signal_handler_block(data->CY_type, data->handler_id[HID_REPDIST_VIEW]);
		hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_type), 1);
		g_signal_handler_unblock(data->CY_type, data->handler_id[HID_REPDIST_VIEW]);
		tmptype = REPORT_TYPE_EXPENSE;
	}

	// define view/sort column
	column = LST_REPDIST_POS; 

	if( byamount )
	{
		switch( tmptype )
		{
			case REPORT_TYPE_ALL: //Inc & Exp
			case REPORT_TYPE_BALANCE:
				column = LST_REPDIST_BALANCE;
				break;
			case REPORT_TYPE_EXPENSE:
				column = LST_REPDIST_EXPENSE;
				break;
			case REPORT_TYPE_INCOME:
				column = LST_REPDIST_INCOME;
				break;
		}
	}

	DB( g_print(" sort on column %d\n\n", column) );

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), column, GTK_SORT_DESCENDING);

	gtk_chart_set_color_scheme(GTK_CHART(data->RE_chart), PREFS->report_color_scheme);

	//TRANSLATORS: example 'Expense by Category'
	title = g_strdup_printf(_("%s by %s"), hbtk_get_label(CYA_REPORT_TYPE,tmptype), hbtk_get_label(CYA_REPORT_SRC, tmpsrc) );

	/* update absolute or not */
	gboolean abs = (tmptype == REPORT_TYPE_EXPENSE || tmptype == REPORT_TYPE_INCOME) ? TRUE : FALSE;
	gtk_chart_set_absolute(GTK_CHART(data->RE_chart), abs);

	/* show xval for month/year and no by amount display */
	xval = FALSE;

	if( !byamount && (tmpsrc == REPORT_SRC_MONTH || tmpsrc == REPORT_SRC_YEAR) )
	{
		xval = TRUE;
		/*switch( tmpsrc)
		{
			case REPORT_SRC_MONTH:
				gtk_chart_set_every_xval(GTK_CHART(data->RE_chart), 4);
				break;
			case REPORT_SRC_YEAR:
				gtk_chart_set_every_xval(GTK_CHART(data->RE_chart), 2);
				break;
		}*/
	}

	gtk_chart_show_xval(GTK_CHART(data->RE_chart), xval);


	/* update bar chart */
	if( tmptype == REPORT_TYPE_ALL ) //dual exp/inc
	{
		DB( g_print(" set bar to dual exp %d/inc %d\n\n", LST_REPDIST_EXPENSE, LST_REPDIST_INCOME) );
		gtk_chart_set_dualdatas(GTK_CHART(data->RE_chart), model, LST_REPDIST_EXPENSE, LST_REPDIST_INCOME, title, NULL);
	}
	else
	{
		// /!\ hazardous here
		column = LST_REPDIST_EXPENSE+(tmptype-1)*2;
		DB( g_print(" set bar to %d\n\n", column) );
		gtk_chart_set_datas(GTK_CHART(data->RE_chart), model, column, title, NULL);
	}


	g_free(title);
	
}


static void ui_repdist_update_date_widget(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;

	DB( g_print("\n[repdist] update date widget\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPDIST_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPDIST_MAXDATE]);
	
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
	
	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPDIST_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPDIST_MAXDATE]);

}


static void ui_repdist_update_daterange(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gchar *daterange;

	DB( g_print("\n[repdist] update daterange\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	daterange = filter_daterange_text_get(data->filter);
	gtk_label_set_markup(GTK_LABEL(data->TX_daterange), daterange);
	g_free(daterange);
}

static void ui_repdist_update_total(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
//gboolean minor;

	DB( g_print("\n[repdist] update total\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	hb_label_set_colvalue(GTK_LABEL(data->TX_total[0]), data->total_expense, GLOBALS->kcur, GLOBALS->minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[1]), data->total_income, GLOBALS->kcur, GLOBALS->minor);
	hb_label_set_colvalue(GTK_LABEL(data->TX_total[2]), data->total_expense + data->total_income, GLOBALS->kcur, GLOBALS->minor);


}


static void ui_repdist_export_result_clipboard(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
GtkClipboard *clipboard;
GString *node;

	DB( g_print("\n[repdist] export result clipboard\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	node = ui_list_repdist_to_string(GTK_TREE_VIEW(data->LV_report), TRUE);

	clipboard = gtk_clipboard_get_default(gdk_display_get_default());
	gtk_clipboard_set_text(clipboard, node->str, node->len);

	g_string_free(node, TRUE);
}


static void ui_repdist_export_result_csv(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gchar *filename = NULL;
GString *node;
GIOChannel *io;
gchar *name;
gint tmpsrc;

	DB( g_print("\n[repdist] export result csv\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	
	name = g_strdup_printf("hb-repstat_%s.csv", hbtk_get_label(CYA_REPORT_SRC,tmpsrc));

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );
		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			node = ui_list_repdist_to_string(GTK_TREE_VIEW(data->LV_report), FALSE);
			g_io_channel_write_chars(io, node->str, -1, NULL, NULL);
			g_io_channel_unref (io);
			g_string_free(node, TRUE);
		}
		g_free( filename );
	}
	g_free(name);
}


static void ui_repdist_export_detail_clipboard(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
GtkClipboard *clipboard;
GString *node;

	DB( g_print("\n[repdist] export detail clipboard\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	node = list_txn_to_string(GTK_TREE_VIEW(data->LV_detail), TRUE);

	clipboard = gtk_clipboard_get_default(gdk_display_get_default());
	gtk_clipboard_set_text(clipboard, node->str, node->len);

	g_string_free(node, TRUE);
}


static void ui_repdist_export_detail_csv(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gchar *filename = NULL;
GString *node;
GIOChannel *io;
gchar *name;
gint tmpsrc;

	DB( g_print("\n[repdist] export detail csv\n") );

	data = user_data;
	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	name = g_strdup_printf("hb-repstat-detail_%s.csv", hbtk_get_label(CYA_REPORT_SRC,tmpsrc));

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


static void ui_repdist_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
guint active = GPOINTER_TO_INT(user_data);
guint tmpsrc;
GList *list;
GtkTreeModel *model;
GtkTreeIter  iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n[repdist] detail\n") );

	/* clear and detach our model */
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if(data->detail)
	{
		//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
		tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));

		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_detail), NULL); /* Detach model from view */

		/* fill in the model */
		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;
		gdouble dtlamt = ope->amount;

			if(filter_txn_match(data->filter, ope) == 1)
			{
			gboolean match = FALSE;
			guint i, pos = 0;

				if( tmpsrc != REPORT_SRC_TAG )
				{
					if( (tmpsrc == REPORT_SRC_CATEGORY || tmpsrc == REPORT_SRC_SUBCATEGORY) && ope->flags & OF_SPLIT )
					{
					guint nbsplit = da_splits_length(ope->splits);
					Split *split;

						dtlamt = 0.0;
						for(i=0;i<nbsplit;i++)
						{
							split = da_splits_get(ope->splits, i);
							pos = category_report_id(split->kcat, (tmpsrc == REPORT_SRC_SUBCATEGORY) ? TRUE : FALSE);
							if( pos == active )
							{
								match = TRUE;
								dtlamt += split->amount;
								// no more break here as we need to compute split 4 cat
								//break;
							}
						}
					}
					else
					{
						pos = report_items_get_pos(tmpsrc, data->filter->mindate, ope);
						if( pos == active )
						{
							match = TRUE;
						}
					}
				}
				else
				/* the TAG process is particular */
				{
					if(ope->tags != NULL)
					{
					guint32 *tptr = ope->tags;

						while(*tptr)
						{
							pos = *tptr - 1;

							DB( g_print(" -> storing tag %d %.2f\n", pos, ope->amount) );

							if( pos == active )
							{
								match = TRUE;
								break;
							}
							tptr++;
						}
					}
				}

				if( match == TRUE )
				{
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


static void ui_repdist_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gint tmpsrc, tmptype;
guint32 from, to;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list, *tmplist = NULL;
guint n_result, sortid;
guint i;
gdouble *tmp_income, *tmp_expense;
gdouble exprate, incrate, balrate;

	DB( g_print("\n[repdist] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//tmpsrc  = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_src));
	tmpsrc  = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_src));
	tmptype = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_type));


	DB( g_print(" for=%d,type=%d\n", tmpsrc, tmptype) );

	//get our min max date
	from = data->filter->mindate;
	to   = data->filter->maxdate;
	if(to < from) return;


	g_queue_free (data->txn_queue);
	data->txn_queue = hbfile_transaction_get_partial(data->filter->mindate, data->filter->maxdate);
	
	DB( g_print(" nb-txn=%d\n", g_queue_get_length (data->txn_queue) ) );

	//todo: remove this later on
	n_result = report_items_count(tmpsrc, data->filter->mindate, data->filter->maxdate);

	switch(tmpsrc)
	{
		case REPORT_SRC_CATEGORY:
		case REPORT_SRC_SUBCATEGORY:
			tmplist = category_glist_sorted(1);
			break;
		case REPORT_SRC_PAYEE:
			tmplist = payee_glist_sorted(1);
			break;
		case REPORT_SRC_ACCOUNT:
			tmplist = account_glist_sorted(1);
			break;
		case REPORT_SRC_TAG:
			tmplist = tag_glist_sorted(1);
			break;
	}

	DB( g_print(" %s :: n_result=%d\n", hbtk_get_label(CYA_REPORT_SRC,tmpsrc), n_result) );

	/* allocate some memory */
	tmp_expense = g_malloc0((n_result+2) * sizeof(gdouble));
	tmp_income  = g_malloc0((n_result+2) * sizeof(gdouble));

	data->total_expense = 0.0;
	data->total_income  = 0.0;

	if(tmp_expense && tmp_income)
	{

		DB( g_print(" - ok memory\n") );

		/* compute the results */
		list = g_queue_peek_head_link(data->txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;
			
			DB( g_print("** testing '%s', cat=%d==> %d\n", ope->memo, ope->kcat, filter_txn_match(data->filter, ope)) );

			if( (filter_txn_match(data->filter, ope) == 1) )
			{
			guint32 pos = 0;
			gdouble trn_amount;

				DB( g_print(" - should insert\n") );

				//trn_amount = ope->amount;
				trn_amount = hb_amount_base(ope->amount, ope->kcur);

				//#1562372 in case of a split we need to take amount for filter categories only
				if( ope->flags & OF_SPLIT )
				{
				guint nbsplit = da_splits_length(ope->splits);
				Split *split;
				Category *catentry;
				gint sinsert;

					trn_amount = 0.0;

					for(i=0;i<nbsplit;i++)
					{
						split = da_splits_get(ope->splits, i);
						catentry = da_cat_get(split->kcat);
						if(catentry == NULL) continue;
						sinsert = ( catentry->flt_select == TRUE ) ? 1 : 0;
						if(data->filter->option[FILTER_CATEGORY] == 2) sinsert ^= 1;

						DB( g_print(" split '%s' insert=%d\n",catentry->name, sinsert) );

						if( (data->filter->option[FILTER_CATEGORY] == 0) || sinsert)
						{
							trn_amount += hb_amount_base(split->amount, ope->kcur);
						}
					}

				}


				if( tmpsrc != REPORT_SRC_TAG )
				{
					if( (tmpsrc == REPORT_SRC_CATEGORY || tmpsrc == REPORT_SRC_SUBCATEGORY) && ope->flags & OF_SPLIT )
					{
					guint nbsplit = da_splits_length(ope->splits);
					Split *split;
					Category *catentry;
					gint sinsert;

						for(i=0;i<nbsplit;i++)
						{
							split = da_splits_get(ope->splits, i);
							catentry = da_cat_get(split->kcat);
							if(catentry == NULL) continue;
							sinsert = ( catentry->flt_select == TRUE ) ? 1 : 0;
							if(data->filter->option[FILTER_CATEGORY] == 2) sinsert ^= 1;

							DB( g_print(" split '%s' insert=%d\n",catentry->name, sinsert) );
							
							if( (data->filter->option[FILTER_CATEGORY] == 0) || sinsert)
							{
								switch(tmpsrc)
								{
									case REPORT_SRC_CATEGORY:
										{
											pos = (catentry->flags & GF_SUB) ? catentry->parent : catentry->key;
										}
										break;
									case REPORT_SRC_SUBCATEGORY:
										pos = split->kcat;
										break;
								}

								trn_amount = hb_amount_base(split->amount, ope->kcur);
								//trn_amount = split->amount;

								if(trn_amount > 0.0)
								{
									tmp_income[pos] += trn_amount;
									data->total_income  += trn_amount;
								}
								else
								{
									tmp_expense[pos] += trn_amount;
									data->total_expense += trn_amount;
								}

							}
							// end insert

						}
					}
					else
					{
						pos = report_items_get_pos(tmpsrc, from, ope);
						if(trn_amount > 0.0)
						{
							tmp_income[pos] += trn_amount;
							data->total_income  += trn_amount;
						}
						else
						{
							tmp_expense[pos] += trn_amount;
							data->total_expense += trn_amount;
						}
					}
				}
				else
				/* the TAG process is particularly */
				{
					if(ope->tags != NULL)
					{
					guint32 *tptr = ope->tags;

						while(*tptr)
						{
							pos = *tptr - 1;

							DB( g_print(" -> storing tag %d %s %.2f\n", pos, da_tag_get(*tptr)->name, trn_amount) );

							if(trn_amount > 0.0)
							{
								tmp_income[pos] += trn_amount;
								data->total_income  += trn_amount;
							}
							else
							{
								tmp_expense[pos] += trn_amount;
								data->total_expense += trn_amount;
							}
							tptr++;
						}
					}
				}

				// fix total according to selection
				//if(tmpkind==0 && !tmp_expense[pos]) { data->total_income  -= ope->amount; }
				//if(tmpkind==1 && !tmp_income[pos] ) { data->total_expense -= ope->amount; }


			}

			list = g_list_next(list);
		}

		/* clear and detach our model */
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL); /* Detach model from view */

		/* insert into the treeview */
		for(i=0, sortid=0; i<n_result; i++)
		{
		gchar *name, *fullcatname;
		gchar buffer[64];
		GDate *date;

			name = NULL;
			fullcatname = NULL;

			DB( g_print("try to insert item %d - %.2f %.2f\n", i, tmp_expense[i], tmp_income[i]) );


			/* filter empty results */
			// 1829630 crash _cairo_arc_in_direction (account was missing here)
			
			if( !(tmpsrc == REPORT_SRC_MONTH || tmpsrc == REPORT_SRC_YEAR) )
			{
				if( tmptype == 1 && !tmp_expense[i] ) continue;
				if( tmptype == 2 && !tmp_income[i] ) continue;
				if( !tmp_expense[i] && !tmp_income[i] ) continue;
			}

			/* get the result name */
			switch(tmpsrc)
			{
				case REPORT_SRC_CATEGORY:
					{
					Category *entry = da_cat_get(i);
						if(entry != NULL)
						{
							name = entry->key == 0 ? _("(no category)") : entry->name;
					
							sortid = g_list_index(tmplist, entry);
						}
					}
					break;

				case REPORT_SRC_SUBCATEGORY:
					{
					Category *entry = da_cat_get(i);
						if(entry != NULL)
						{
							if(entry->flags & GF_SUB)
							{
							Category *parent = da_cat_get(entry->parent);

								fullcatname = g_strdup_printf("%s : %s", parent->name, entry->name);
								name = fullcatname;
							}
							else
								name = entry->key == 0 ? _("(no category)") : entry->name;
								
							sortid = g_list_index(tmplist, entry);
						}
					}
					break;

				case REPORT_SRC_PAYEE:
					{
					Payee *entry = da_pay_get(i);
						if(entry != NULL)
						{
							name = entry->key == 0 ? _("(no payee)") : entry->name;
							sortid = g_list_index(tmplist, entry);
						}
					}
					break;

				case REPORT_SRC_ACCOUNT:
					{
					Account *entry = da_acc_get(i);
						if(entry != NULL)
						{
							name = entry->name;
							sortid = g_list_index(tmplist, entry);
						}
					}
					break;

				case REPORT_SRC_TAG:
					{
					Tag *entry = da_tag_get(i+1);
						name = entry->name;
						sortid = g_list_index(tmplist, entry);
					}
					break;

				case REPORT_SRC_MONTH:
					date = g_date_new_julian(from);
					g_date_add_months(date, i);
					//g_snprintf(buffer, 63, "%d-%02d", g_date_get_year(date), g_date_get_month(date));
					g_snprintf(buffer, 63, "%d-%s", g_date_get_year(date), _(CYA_ABMONTHS[g_date_get_month(date)]));
					g_date_free(date);
					name = buffer;
					break;

				case REPORT_SRC_YEAR:
					date = g_date_new_julian(from);
					g_date_add_years(date, i);
					g_snprintf(buffer, 63, "%d", g_date_get_year(date));
					g_date_free(date);
					name = buffer;
					break;
			}

			DB( g_print(" inserting %2d, '%s', %9.2f %9.2f %9.2f\n", i, name, tmp_expense[i], tmp_income[i], tmp_expense[i] + tmp_income[i]) );

			//compute rate
			exprate = 0.0;
			incrate = 0.0;
			balrate = 0.0;

			if( data->total_expense )
				exprate = ABS((tmp_expense[i] * 100 / data->total_expense));

			if( data->total_income )
				incrate = (tmp_income[i] * 100 / data->total_income);

			data->total_balance = ABS(data->total_expense) + data->total_income; 
			if( data->total_balance )
				balrate = (ABS(tmp_expense[i]) + tmp_income[i]) * 100 / data->total_balance;

	    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
     		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_REPDIST_POS, sortid++,
				LST_REPDIST_KEY, i,
				LST_REPDIST_NAME, name,
				LST_REPDIST_EXPENSE, tmp_expense[i],
				LST_REPDIST_INCOME , tmp_income[i],
				LST_REPDIST_BALANCE, tmp_expense[i] + tmp_income[i],
				LST_REPDIST_EXPRATE, exprate,
				LST_REPDIST_INCRATE, incrate,
				LST_REPDIST_BALRATE, balrate,
				-1);

			g_free(fullcatname);
		}

		/* update column 0 title */
		GtkTreeViewColumn *column = gtk_tree_view_get_column( GTK_TREE_VIEW(data->LV_report), 0);
		gtk_tree_view_column_set_title(column, hbtk_get_label(CYA_REPORT_SRC,tmpsrc));

		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));
		
		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
		g_object_unref(model);
	}

	/* free our memory */
	g_free(tmp_expense);
	g_free(tmp_income);

	/* free tmplist (sort cat/pay) */
	g_list_free(tmplist);

	ui_repdist_update_total(widget,NULL);

	ui_repdist_update(widget, user_data);

}


/*
** update sensitivity
*/
static void ui_repdist_sensitive(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
GtkAction *action;
gboolean visible, sensitive;
gint page;

	DB( g_print("\n[repdist] sensitive\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->GR_result));

	visible = page == 0 ? TRUE : FALSE;
	action = gtk_ui_manager_get_action(data->ui, "/ToolBar/Detail");
	gtk_action_set_visible (action, visible);
	//sensitive = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), NULL, NULL);
	//gtk_action_set_sensitive(action, sensitive);
	//action = gtk_ui_manager_get_action(data->ui, "/ToolBar/Export");
	//gtk_action_set_visible (action, visible);
	hb_widget_visible (data->BT_export, visible);


	visible = page == 0 ? FALSE : TRUE;
	//todo: don't display for pie chart (get the type form chart)
	
	hb_widget_visible(data->LB_zoomx, visible);
	hb_widget_visible(data->RG_zoomx, visible);

	visible = page == 0 ? FALSE : TRUE;
	gtk_action_set_visible(gtk_ui_manager_get_action(data->ui, "/ToolBar/Legend"), visible);

	visible = page == 0 ? TRUE : FALSE;
	gtk_action_set_visible(gtk_ui_manager_get_action(data->ui, "/ToolBar/Rate"), visible);
	
	sensitive = gtk_tree_model_iter_n_children(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail)), NULL) > 0 ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->MI_detailtoclip, sensitive);
	gtk_widget_set_sensitive(data->MI_detailtocsv, sensitive);
}


static void ui_repdist_detail_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
struct ui_repdist_data *data;
Transaction *active_txn;
gboolean result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print ("\n[repdist] A detail row has been double-clicked!\n") );

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
			ui_repdist_compute(data->window, NULL);
		}

		da_transaction_free (old_txn);
	}
}


static void ui_repdist_update_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

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
				gtk_tree_model_get(model, &iter, LST_REPDIST_KEY, &key, -1);

				DB( g_print(" - active is %d\n", key) );

				ui_repdist_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
			}



			gtk_widget_show(data->GR_detail);
		}
		else
			gtk_widget_hide(data->GR_detail);

	}
}




static void ui_repdist_toggle_detail(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->detail ^= 1;

	DB( g_print("\n[repdist] toggledetail to %d\n", data->detail) );

	ui_repdist_update_detail(widget, user_data);

}

/*
** change the chart legend visibility
*/
static void ui_repdist_toggle_legend(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
//gint active;

	DB( g_print("\n[repdist] toggle legend\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->legend ^= 1;

	gtk_chart_show_legend(GTK_CHART(data->RE_chart), data->legend, FALSE);

}

static void ui_repdist_zoomx_callback(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
gdouble value;

	DB( g_print("\n[repdist] zoomx\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_range_get_value(GTK_RANGE(data->RG_zoomx));

	DB( g_print(" + scale is %.2f\n", value) );

	gtk_chart_set_barw(GTK_CHART(data->RE_chart), value);

}


/*
** change the chart rate columns visibility
*/
static void ui_repdist_toggle_rate(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;
GtkTreeViewColumn *column;

	DB( g_print("\n[repdist] toggle rate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->rate ^= 1;

	if(GTK_IS_TREE_VIEW(data->LV_report))
	{

		column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 2);
		gtk_tree_view_column_set_visible(column, data->rate);

		column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 4);
		gtk_tree_view_column_set_visible(column, data->rate);

		column = gtk_tree_view_get_column (GTK_TREE_VIEW(data->LV_report), 6);
		gtk_tree_view_column_set_visible(column, data->rate);
	}

}

static void ui_repdist_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct ui_repdist_data *data;

	DB( g_print("\n[repdist] toggle minor\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));

	ui_repdist_update_total(widget,NULL);

	//hbfile_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));
	gtk_chart_show_minor(GTK_CHART(data->RE_chart), GLOBALS->minor);

}


/*
**
*/
static void ui_repdist_setup(struct ui_repdist_data *data)
{
	DB( g_print("\n[repdist] setup\n") );

	data->txn_queue = g_queue_new ();

	data->filter = da_flt_malloc();
	filter_reset(data->filter);


	data->detail = PREFS->stat_showdetail;
	data->legend = 1;
	data->rate = PREFS->stat_showrate^1;


	ui_repdist_toggle_rate(data->window, NULL);



	/* 3.4 : make int transfer out of stats */
	data->filter->option[FILTER_PAYMODE] = 1;
	data->filter->paymode[PAYMODE_INTXFER] = FALSE;

	filter_preset_daterange_set(data->filter, PREFS->date_range_rep, 0);
	
	ui_repdist_update_date_widget(data->window, NULL);

}



static void ui_repdist_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
GtkTreeModel *model;
GtkTreeIter iter;
guint key = -1;

	DB( g_print("\n[repdist] selection\n") );

	if (gtk_tree_selection_get_selected(treeselection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_REPDIST_KEY, &key, -1);
	}

	DB( g_print(" - active is %d\n", key) );

	ui_repdist_detail(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), GINT_TO_POINTER(key));
	ui_repdist_sensitive(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


/*
**
*/
static gboolean ui_repdist_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct ui_repdist_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("\n[repdist] dispose\n") );

	g_queue_free (data->txn_queue);

	da_flt_free(data->filter);

	g_free(data);

	//store position and size
	wg = &PREFS->sta_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);

	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );



	//enable define windows
	GLOBALS->define_off--;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

	return FALSE;
}


// the window creation
GtkWidget *ui_repdist_window_new(void)
{
struct ui_repdist_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *notebook, *treeview, *vpaned, *sw;
GtkWidget *label, *widget, *table, *entry;
gint row;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	DB( g_print("\n[repdist] new\n") );

	
	data = g_malloc0(sizeof(struct ui_repdist_data));
	if(!data) return NULL;

	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" - new window=%p, inst_data=%p\n", window, data) );

	gtk_window_set_title (GTK_WINDOW (window), _("Statistics Report"));
	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_REP_STATS);


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
	//widget = make_cycle(label, CYA_REPORT_SRC);
	widget = hbtk_combo_box_new_with_data(label, CYA_REPORT_SRC);
	data->CY_src = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Type:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	//widget = make_cycle(label, CYA_REPORT_TXN_TYPE);
	widget = hbtk_combo_box_new_with_data(label, CYA_REPORT_TYPE);
	data->CY_type = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);


	row++;
	widget = gtk_check_button_new_with_mnemonic (_("By _amount"));
	data->CM_byamount = widget;
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

/*
	row++;
	widget = gtk_check_button_new_with_mnemonic ("Legend");
	data->CM_legend = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, 2, row, row+1);
*/
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
	gtk_action_group_add_radio_actions (actions, radio_entries, n_radio_entries, 0, G_CALLBACK(ui_repdist_action_mode), data);

	gtk_action_group_add_actions (actions, entries, n_entries, data);

	gtk_action_group_add_toggle_actions (actions,
					   toggle_entries, n_toggle_entries,
					   data);


	/* set which action should have priority in the toolbar */
	//action = gtk_action_group_get_action(actions, "List");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Column");
	//g_object_set(action, "is_important", TRUE, NULL);

	//action = gtk_action_group_get_action(actions, "Donut");
	//g_object_set(action, "is_important", TRUE, NULL);

	action = gtk_action_group_get_action(actions, "Detail");
	//g_object_set(action, "is_important", TRUE, NULL);
	g_object_set(action, "active", PREFS->stat_showdetail, NULL);

	action = gtk_action_group_get_action(actions, "Rate");
	g_object_set(action, "active", PREFS->stat_showrate, NULL);

	//action = gtk_action_group_get_action(actions, "Filter");
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
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_repdist_export_result_clipboard), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Result to CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_repdist_export_result_csv), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Detail to clipboard"));
	data->MI_detailtoclip = menuitem;
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_repdist_export_detail_clipboard), data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Detail to CSV"));
	data->MI_detailtocsv = menuitem;
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_repdist_export_detail_csv), data);

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


	//infos + balance
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
	label = gtk_label_new(_("Balance:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	entry = gtk_label_new(NULL);
	data->TX_total[1] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Income:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	entry = gtk_label_new(NULL);
	data->TX_total[0] = entry;
	gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	label = gtk_label_new(_("Expense:"));
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);


	/* report area */
	notebook = gtk_notebook_new();
	data->GR_result = notebook;
	gtk_widget_show(notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE, 0);

	// page list/detail
	vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vpaned, NULL);

	// list
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = ui_list_repdist_create();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(sw), treeview);
	gtk_paned_pack1 (GTK_PANED(vpaned), sw, TRUE, TRUE);

	//detail
	sw = gtk_scrolled_window_new (NULL, NULL);
	data->GR_detail = sw;
	//gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (sw), GTK_CORNER_TOP_RIGHT);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	treeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	data->LV_detail = treeview;
	gtk_container_add (GTK_CONTAINER(sw), treeview);
	gtk_paned_pack2 (GTK_PANED(vpaned), sw, TRUE, TRUE);	

	//page: 2d bar /pie
	widget = gtk_chart_new(CHART_TYPE_COL);
	data->RE_chart = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.symbol);
	gtk_chart_set_currency(GTK_CHART(widget), GLOBALS->kcur);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), widget, NULL);

	//todo: setup should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor), GLOBALS->minor);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_byamount), PREFS->stat_byamount);

	hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_type), REPORT_TYPE_EXPENSE);

	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_detail))), "minor", (gpointer)data->CM_minor);

	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (ui_repdist_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (ui_repdist_toggle_minor), NULL);

    data->handler_id[HID_REPDIST_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (ui_repdist_date_change), (gpointer)data);
    data->handler_id[HID_REPDIST_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (ui_repdist_date_change), (gpointer)data);

	data->handler_id[HID_REPDIST_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (ui_repdist_range_change), NULL);

    g_signal_connect (data->CY_src, "changed", G_CALLBACK (ui_repdist_compute), (gpointer)data);

    data->handler_id[HID_REPDIST_VIEW] = g_signal_connect (data->CY_type, "changed", G_CALLBACK (ui_repdist_compute), (gpointer)data);

	g_signal_connect (data->RG_zoomx, "value-changed", G_CALLBACK (ui_repdist_zoomx_callback), NULL);


	g_signal_connect (data->CM_byamount, "toggled", G_CALLBACK (ui_repdist_update), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_report)), "changed", G_CALLBACK (ui_repdist_selection), NULL);

	g_signal_connect (GTK_TREE_VIEW(data->LV_detail), "row-activated", G_CALLBACK (ui_repdist_detail_onRowActivated), NULL);

	//setup, init and show window
	ui_repdist_setup(data);

	/* toolbar */
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);

	
	//setup, init and show window
	wg = &PREFS->sta_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);



	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);

	//gtk_widget_hide(data->GR_detail);



	ui_repdist_sensitive(window, NULL);
	ui_repdist_update_detail(window, NULL);

	DB( g_print("range: %d\n", PREFS->date_range_rep) );

	if( PREFS->date_range_rep != 0)
		hbtk_combo_box_set_active_id(GTK_COMBO_BOX_TEXT(data->CY_range), PREFS->date_range_rep);
	else
		ui_repdist_compute(window, NULL);


	return window;
}

/*
** ============================================================================
*/


static GString *ui_list_repdist_to_string(GtkTreeView *treeview, gboolean clipboard)
{
GString *node;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
const gchar *format;

	node = g_string_new(NULL);

	// header
	format = (clipboard == TRUE) ? "%s\t%s\t%s\t%s\n" : "%s;%s;%s;%s\n";
	g_string_append_printf(node, format, _("Result"), _("Expense"), _("Income"), _("Balance"));

	model = gtk_tree_view_get_model(treeview);
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	gchar *name;
	gdouble exp, inc, bal;

		gtk_tree_model_get (model, &iter,
			//LST_REPDIST_KEY, i,
			LST_REPDIST_NAME   , &name,
			LST_REPDIST_EXPENSE, &exp,
			LST_REPDIST_INCOME , &inc,
			LST_REPDIST_BALANCE, &bal,
			-1);

		format = (clipboard == TRUE) ? "%s\t%.2f\t%.2f\t%.2f\n" : "%s;%.2f;%.2f;%.2f\n";
		g_string_append_printf(node, format, name, exp, inc, bal);

		//leak
		g_free(name);
		
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	//DB( g_print("text is:\n%s", node->str) );

	return node;
}




static void ui_list_repdist_rate_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
   {
     //GtkWidget *widget;

     //widget = g_object_get_data(G_OBJECT(model), "minor");

	//todo g_assert here and null test

     gdouble  tmp;
     gchar   buf[128];

	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &tmp, -1);

	if(tmp != 0.0)
	{
		g_snprintf(buf, sizeof(buf), "%.2f %%", tmp);
		g_object_set(renderer, "text", buf, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);

}


static void ui_list_repdist_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

	gtk_tree_model_get(model, iter, GPOINTER_TO_INT(user_data), &value, -1);

	if( value )
	{
		hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, value, GLOBALS->kcur, GLOBALS->minor);

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


static GtkTreeViewColumn *ui_list_repdist_amount_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_list_repdist_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *ui_list_repdist_rate_column(gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "%");
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, "yalign", 1.0, "scale", 0.8, "scale-set", TRUE, NULL);
	
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", id);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_list_repdist_rate_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);

	//gtk_tree_view_column_set_visible(column, FALSE);

	return column;
}

/*
** create our statistic list
*/
static GtkWidget *ui_list_repdist_create(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_REPDIST,
		G_TYPE_INT,			//keep for compatibility with chart
	    	G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);

	/* column: Name */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Result"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, ope_result_cell_data_function, NULL, NULL);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_REPDIST_NAME);
	//gtk_tree_view_column_set_sort_column_id (column, LST_REPDIST_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Expense */
	column = ui_list_repdist_amount_column(_("Expense"), LST_REPDIST_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = ui_list_repdist_rate_column(LST_REPDIST_EXPRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Income */
	column = ui_list_repdist_amount_column(_("Income"), LST_REPDIST_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = ui_list_repdist_rate_column(LST_REPDIST_INCRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Balance */
	column = ui_list_repdist_amount_column(_("Balance"), LST_REPDIST_BALANCE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	column = ui_list_repdist_rate_column(LST_REPDIST_BALRATE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPDIST_POS    , ui_list_repdist_compare_func, GINT_TO_POINTER(LST_REPDIST_POS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPDIST_EXPENSE, ui_list_repdist_compare_func, GINT_TO_POINTER(LST_REPDIST_EXPENSE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPDIST_INCOME , ui_list_repdist_compare_func, GINT_TO_POINTER(LST_REPDIST_INCOME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_REPDIST_BALANCE, ui_list_repdist_compare_func, GINT_TO_POINTER(LST_REPDIST_BALANCE), NULL);


	return(view);
}

static gint ui_list_repdist_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint sortcol = GPOINTER_TO_INT(userdata);
gint retval = 0;
gint pos1, pos2;
gdouble val1, val2;

	gtk_tree_model_get(model, a,
		LST_REPDIST_POS, &pos1,
		sortcol, &val1,
		-1);
	gtk_tree_model_get(model, b,
		LST_REPDIST_POS, &pos2,
		sortcol, &val2,
		-1);

	switch(sortcol)
	{
		case LST_REPDIST_POS:
			retval = pos2 - pos1;
			break;
		default:
			retval = (ABS(val1) - ABS(val2)) > 0 ? 1 : -1;
			break;
	}

	//DB( g_print(" sort %d=%d or %.2f=%.2f :: %d\n", pos1,pos2, val1, val2, ret) );

    return retval;
  }

