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

#include "dsp-mainwindow.h"

#include "hub-spending.h"
#include "gtk-chart.h"


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


extern gchar *CYA_CATSUBCAT[];


static GtkWidget *create_list_topspending(void)
{
GtkListStore *store;
GtkWidget *view;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_TOPSPEND,
		G_TYPE_INT,
		G_TYPE_INT,
		G_TYPE_STRING,	//category
		G_TYPE_DOUBLE,	//amount
		G_TYPE_INT		//rate
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	return(view);
}




static gint tmptop_compare_func(struct tmptop *tt1, struct tmptop *tt2)
{
	return tt1->value > tt2->value ? 1 : -1;
}


void ui_hub_spending_update(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
GtkTreeModel *model;
gchar *title;
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];

	DB( g_print("\n[hub-spendings] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	hb_strfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, data->toptotal, GLOBALS->kcur, GLOBALS->minor);	
	//hb_label_set_amount(GTK_LABEL(data->TX_topamount), total, GLOBALS->kcur, GLOBALS->minor);
	title = g_strdup_printf("%s %s", _("Top spending"), strbuffer);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_top));
	
	gtk_chart_set_color_scheme(GTK_CHART(data->RE_pie), PREFS->report_color_scheme);
	gtk_chart_set_currency(GTK_CHART(data->RE_pie), GLOBALS->kcur);
	gtk_chart_set_datas(GTK_CHART(data->RE_pie), model, LST_TOPSPEND_AMOUNT, title, NULL);

	g_free(title);

	//future usage
	gchar *fu = _("Top %d spending"); title = fu;
}


void ui_hub_spending_populate(GtkWidget *widget, gpointer user_data)
{
struct hbfile_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gint type, range;
guint n_result, i, n_items;
GArray *garray;
gdouble total, other;
Account *acc;

	DB( g_print("\n[hub-spendings] populate\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	type  = hbtk_radio_button_get_active(GTK_CONTAINER(data->RA_type));
	range = hbtk_combo_box_get_active_id(GTK_COMBO_BOX_TEXT(data->CY_range));

	DB( g_print(" - type=%d, range=%d\n", type, range) );
	DB( g_print(" - pref range=%d\n", PREFS->date_range_wal) );

	if(range == FLT_RANGE_OTHER)
		return;
	
	filter_preset_daterange_set(data->filter, range, 0);
	
	
	n_result = da_cat_get_max_key() + 1;
	total = 0.0;

	DB( g_print(" - max key is %d\n", n_result) );

	/* allocate some memory */
	garray = g_array_sized_new(FALSE, FALSE, sizeof(struct tmptop), n_result);

	if(garray)
	{
	struct tmptop zero = { .key=0, .value=0.0 };
	GQueue *txn_queue;
		
		//DB( g_print(" - array length=%d\n", garray->len) );

		for(i=0 ; i<n_result ; i++)
		{
			g_array_append_vals(garray, &zero, 1);
			//g_array_insert_vals(garray, i, &zero, 1);

			//struct tmptop *tt = &g_array_index (garray, struct tmptop, i);
			//DB( g_print("%4d, %4d %f\n", i, tt->key, tt->value) );
		}

		//DB( g_print("\n - end array length=%d\n", garray->len) );

		//todo: not ideal, has ot force to get_acc for each txn below
		txn_queue = hbfile_transaction_get_partial(data->filter->mindate, data->filter->maxdate);

		/* compute the results */
		list = g_queue_peek_head_link(txn_queue);
		while (list != NULL)
		{
		Transaction *ope = list->data;

			//DB( g_print(" - eval txn: '%s', cat=%d ==> flt-test=%d\n", ope->memo, ope->kcat, filter_txn_match(data->filter, ope)) );

			if( !(ope->paymode == PAYMODE_INTXFER) )
			{
			guint32 pos = 0;
			gdouble trn_amount;

				//todo: optimize here
				trn_amount = ope->amount;
				acc = da_acc_get(ope->kacc);
				if(acc)
					trn_amount = hb_amount_base(ope->amount, acc->kcur);

				if( ope->flags & OF_SPLIT )
				{
				guint nbsplit = da_splits_length(ope->splits);
				Split *split;
				struct tmptop *item;
				
					for(i=0;i<nbsplit;i++)
					{
						split = da_splits_get(ope->splits, i);
						pos = category_report_id(split->kcat, type);
						if( pos <= garray->len )
						{
							trn_amount = hb_amount_base(split->amount, acc->kcur);
							//trn_amount = split->amount;
							//#1297054 if( trn_amount < 0 ) {
								item = &g_array_index (garray, struct tmptop, pos);
								item->key = pos;
								item->value += trn_amount;
								//DB( g_print(" - stored %.2f to item %d\n", trn_amount, pos)  );
							//}
						}
					}
				}
				else
				{
				struct tmptop *item;

					pos = category_report_id(ope->kcat, type);
					if( pos <= garray->len )
					{
						//#1297054 if( trn_amount < 0 ) {
							item = &g_array_index (garray, struct tmptop, pos);
							item->key = pos;
							item->value += trn_amount;
							//DB( g_print(" - stored %.2f to item %d\n", trn_amount, pos)  );
						//}
					}
				}

			}

			list = g_list_next(list);
		}

		g_queue_free (txn_queue);
		
		// we need to sort this and limit before
		g_array_sort(garray, (GCompareFunc)tmptop_compare_func);

		n_items = MIN(garray->len,MAX_TOPSPENDING);
		other = 0;
		for(i=0 ; i<garray->len ; i++)
		{
		struct tmptop *item;
		
			item = &g_array_index (garray, struct tmptop, i);
			if(item->value < 0)
			{
				total += item->value;

				if(i >= n_items)
					other += item->value;

				DB( g_print(" - %d : k='%d' v='%f' t='%f'\n", i, item->key, item->value, total) );

			}
		}

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_top));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_top), NULL); /* Detach model from view */

		/* insert into the treeview */
		for(i=0 ; i<MIN(garray->len,MAX_TOPSPENDING) ; i++)
		{
		gchar *name;
		Category *entry;
		struct tmptop *item;
		gdouble value;
		
			item = &g_array_index (garray, struct tmptop, i);

			if(!item->value) continue;
			//#1767659 top spending should restrict to... spending
			if(item->value < 0)
			{
				value = hb_amount_round(item->value, 2);
				entry = da_cat_get(item->key);
				if(entry == NULL) continue;

				name = da_cat_get_name (entry);

				// append test
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					  LST_TOPSPEND_ID, i,
					  LST_TOPSPEND_KEY, 0,
					  LST_TOPSPEND_NAME, name,
					  LST_TOPSPEND_AMOUNT, value,
					  //LST_TOPSPEND_RATE, (gint)(((ABS(value)*100)/ABS(total)) + 0.5),
					  -1);
			}
		}

		// append test
		if(ABS(other) > 0)
		{
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				  LST_TOPSPEND_ID, n_items,
				  LST_TOPSPEND_KEY, 0,
				  LST_TOPSPEND_NAME, _("Other"),
				  LST_TOPSPEND_AMOUNT, other,
				  //LST_TOPSPEND_RATE, (gint)(((ABS(other)*100)/ABS(total)) + 0.5),
				  -1);
		}
			
		/* Re-attach model to view */
  		gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_top), model);
		g_object_unref(model);
		
		
		// update chart and widgets
		{
		gchar *daterange;

			data->toptotal = total;
			ui_hub_spending_update(widget, data);
			
			daterange = filter_daterange_text_get(data->filter);
			gtk_widget_set_tooltip_markup(GTK_WIDGET(data->CY_range), daterange);
			g_free(daterange);
		}
	}
	
	/* free our memory */
	g_array_free (garray, TRUE);

}



GtkWidget *ui_hub_spending_create(struct hbfile_data *data)
{
GtkWidget *hub, *hbox, *tbar;
GtkWidget *label, *widget;
GtkToolItem *toolitem;

	DB( g_print("\n[hub-spendings] create\n") );
	
	widget = (GtkWidget *)create_list_topspending();
	data->LV_top = widget;

	hub = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hub), SPACING_SMALL);
	data->GR_top = hub;

	/* chart + listview */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (hub), hbox, TRUE, TRUE, 0);

	widget = gtk_chart_new(CHART_TYPE_PIE);
	data->RE_pie = widget;
	gtk_chart_set_minor_prefs(GTK_CHART(widget), PREFS->euro_value, PREFS->minor_cur.symbol);
	gtk_chart_show_legend(GTK_CHART(data->RE_pie), TRUE, TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

	//list toolbar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(tbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_ICONS);
	gtk_style_context_add_class (gtk_widget_get_style_context (tbar), GTK_STYLE_CLASS_INLINE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (hub), tbar, FALSE, FALSE, 0);

	label = make_label_group(_("Where your money goes"));
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), label);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, TRUE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	/* total + date range */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	data->CY_range = make_daterange(label, DATE_RANGE_CUSTOM_HIDE);
	gtk_box_pack_end (GTK_BOX (hbox), data->CY_range, FALSE, FALSE, 0);

	widget = hbtk_radio_button_new(CYA_CATSUBCAT, TRUE);
	data->RA_type = widget;
	gtk_box_pack_end (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	hbtk_radio_button_connect (GTK_CONTAINER(data->RA_type), "toggled", G_CALLBACK (ui_hub_spending_populate), &data);

	g_signal_connect (data->CY_range, "changed", G_CALLBACK (ui_hub_spending_populate), NULL);
	
	return hub;
}
