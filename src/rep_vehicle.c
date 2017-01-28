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

#include "rep_vehicle.h"

#include "list_operation.h"
#include "gtk-chart.h"
#include "gtk-dateentry.h"

#include "dsp_mainwindow.h"
#include "ui-category.h"


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
static void repvehicle_action_refresh(GtkAction *action, gpointer user_data);
static void repvehicle_action_export(GtkAction *action, gpointer user_data);

static void repvehicle_export_csv(GtkWidget *widget, gpointer user_data);
static void repvehicle_date_change(GtkWidget *widget, gpointer user_data);
static void repvehicle_range_change(GtkWidget *widget, gpointer user_data);
static void repvehicle_compute(GtkWidget *widget, gpointer user_data);
static void repvehicle_update(GtkWidget *widget, gpointer user_data);
static void repvehicle_toggle_minor(GtkWidget *widget, gpointer user_data);
static void repvehicle_setup(struct repvehicle_data *data);
static gboolean repvehicle_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static GtkWidget *create_list_repvehicle(void);


static GtkActionEntry entries[] = {
  { "Refresh" , ICONNAME_REFRESH   , N_("Refresh"), NULL,   N_("Refresh results"), G_CALLBACK (repvehicle_action_refresh) },

  { "Export" , ICONNAME_HB_FILE_EXPORT, N_("Export")  , NULL,   N_("Export as CSV"), G_CALLBACK (repvehicle_action_export) },
};
static guint n_entries = G_N_ELEMENTS (entries);

static const gchar *ui_info =
"<ui>"
"  <toolbar name='ToolBar'>"
"    <toolitem action='Refresh'/>"
"      <separator/>"
"    <toolitem action='Export'/>"
"  </toolbar>"
"</ui>";

static void repvehicle_date_change(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;

	DB( g_print("(repvehicle) date change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
	data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	// set min/max date for both widget
	gtk_date_entry_set_maxdate(GTK_DATE_ENTRY(data->PO_mindate), data->filter->maxdate);
	gtk_date_entry_set_mindate(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->mindate);

	g_signal_handler_block(data->CY_range, data->handler_id[HID_REPVEHICLE_RANGE]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), 11);
	g_signal_handler_unblock(data->CY_range, data->handler_id[HID_REPVEHICLE_RANGE]);


	repvehicle_compute(widget, NULL);

}


static void repvehicle_action_refresh(GtkAction *action, gpointer user_data)
{
struct repvehicle_data *data = user_data;

	repvehicle_compute(data->window, NULL);
}

static void repvehicle_action_export(GtkAction *action, gpointer user_data)
{
struct repvehicle_data *data = user_data;

	repvehicle_export_csv(data->window, NULL);
}



static void repvehicle_range_change(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
gint range;

	DB( g_print("(repvehicle) range change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	range = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_range));

	if(range != 11)
	{
		filter_preset_daterange_set(data->filter, range, 0);

		g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPVEHICLE_MINDATE]);
		g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPVEHICLE_MAXDATE]);

		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		
		g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPVEHICLE_MINDATE]);
		g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPVEHICLE_MAXDATE]);

		repvehicle_compute(widget, NULL);
	}
}

static gint repvehicle_transaction_compare_func(CarCost *a, CarCost *b)
{
gint retval;

	//retval = (gint)(a->ope->date - b->ope->date);
	//if( retval == 0 )
		retval = a->meter - b->meter;

	return retval;
}


//#1277622
static CarCost *repvehicle_eval_memofield(CarCost *item, gchar *text)
{
gchar *d, *v1, *v2;
gint len;

	if( text != NULL)
	{
		len = strlen(text);
		d = g_strstr_len(text, len, "d=");
		v1 = g_strstr_len(text, len, "v=");
		v2 = g_strstr_len(text, len, "v~");
		if(d && (v1 || v2))
		{
			item->meter	= atol(d+2);
			if(v1)
			{
				item->fuel	= g_strtod(v1+2, NULL);
				item->partial = FALSE;
			}
			else
			{
				item->fuel	= g_strtod(v2+2, NULL);
				item->partial = TRUE;
			}
		}
	}
	
	return item;
}


static void repvehicle_export_csv(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gchar *filename = NULL;
GIOChannel *io;
gchar *outstr, *name;

	DB( g_print("\n[repvehicle] export csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	name = "hb-vehicle.csv";

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, name) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		io = g_io_channel_new_file(filename, "w", NULL);
		if(io != NULL)
		{
			// header
			outstr = g_strdup_printf("%s;%s;%s;%s;%s;%s;%s;%s\n", 
			_("Date"),
			_("Meter"),
			_("Fuel"),
			_("Price"),
			_("Amount"),
			_("Dist."),
			PREFS->vehicle_unit_100,
			PREFS->vehicle_unit_distbyvol
			);
			g_io_channel_write_chars(io, outstr, -1, NULL, NULL);


			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
			valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			guint32 julian;
			gint meter, dist, distbyvol;
			gdouble fuel, price, amount, centkm;
			gboolean partial;
			gchar datebuf[16];

				gtk_tree_model_get (model, &iter,
					LST_CAR_DATE    , &julian,
					LST_CAR_METER   , &meter,
					LST_CAR_FUEL    , &fuel,
					LST_CAR_PRICE   , &price,
					LST_CAR_AMOUNT  , &amount,
					LST_CAR_DIST    , &dist,
					LST_CAR_100KM   , &centkm,
				    LST_CAR_DISTBYVOL, &distbyvol,
				    LST_CAR_PARTIAL, &partial,
                    -1);
				
				hb_sprint_date(datebuf, julian);

				outstr = g_strdup_printf("%s;%d;%.2f;%.2f;%.2f;%d;%.2f;%d;%d\n", 
					datebuf, meter, fuel, price, amount, dist, centkm, distbyvol, partial);
				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				DB( g_print("%s", outstr) );

				g_free(outstr);

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}

			g_io_channel_unref (io);
		}

		g_free( filename );
	}
}


static void repvehicle_compute(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GList *list;
guint32 catkey;

	DB( g_print("(repvehicle) compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// clear the glist
	da_vehiclecost_destroy(data->vehicle_list);
	data->vehicle_list = NULL;

	// get the category key
	catkey = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_cat));
	
	/* do nothing if cat is 0 */
	if( catkey == 0 )
		goto end1;

	DB( g_print(" -> active cat is %d\n", catkey) );

	g_queue_free (data->txn_queue);
	data->txn_queue = hbfile_transaction_get_partial(data->filter->mindate, data->filter->maxdate);

	// collect transactions
	// the purpose here is to collect all cat transaction
	// and precompute some datas
	list = g_queue_peek_head_link(data->txn_queue);
	while (list != NULL)
	{
	Transaction *ope = list->data;
	Category *cat;
	CarCost *item;

		// eval normal transaction
		if(!(ope->flags & OF_SPLIT))
		{
			cat = da_cat_get(ope->kcat);
			if( cat && (cat->key == catkey || cat->parent == catkey) )
			{
				item = da_vehiclecost_malloc();
				item->date = ope->date;
				item->wording = ope->wording;
				// get amount in base currency
				//item->amount = ope->amount;
				item->amount = hb_amount_base(ope->amount, ope->kcur);

				item = repvehicle_eval_memofield(item, ope->wording);
				data->vehicle_list = g_list_append(data->vehicle_list, item);
				DB( g_print(" -> store acc=%d '%s' %.2f\n", ope->kacc, ope->wording, ope->amount) );
			}
		}
		// eval split transaction
		else
		{
		guint i, nbsplit = da_splits_count(ope->splits);
		Split *split;

			DB( g_print(" -> nb split %d\n", nbsplit) );
			
			for(i=0;i<nbsplit;i++)
			{
				split = ope->splits[i];
				cat = da_cat_get(split->kcat);

				DB( g_print(" -> eval split '%s'\n", split->memo) );
				
				if( cat && (cat->key == catkey || cat->parent == catkey) )
				{
					item = da_vehiclecost_malloc();
					item->date = ope->date;
					item->wording = split->memo;
					// get amount in base currency
					//item->amount = split->amount;
					item->amount = hb_amount_base(split->amount, ope->kcur);

					item = repvehicle_eval_memofield(item, split->memo);
					data->vehicle_list = g_list_append(data->vehicle_list, item);
					DB( g_print(" -> store split part acc=%d '%s' %.2f\n", ope->kacc, split->memo, split->amount) );
				}
			}
		}

		list = g_list_next(list);
	}

	// sort by meter #399170
	data->vehicle_list = g_list_sort(data->vehicle_list, (GCompareFunc)repvehicle_transaction_compare_func);

end1:
	repvehicle_update(widget, NULL);
}


static void repvehicle_update(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;
GtkTreeModel *model;
GtkTreeIter  iter;
GList *list;
gchar *buf;
gint nb_refuel = 0;
guint lastmeter = 0;

	DB( g_print("(repvehicle) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// clear and detach our model
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_object_ref(model);
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), NULL);

	data->total_misccost = 0;
	data->total_fuelcost = 0;
	data->total_fuel	 = 0;
	data->total_dist	 = 0;

	gdouble partial_fuel = 0;
	guint	partial_dist = 0;


	list = g_list_first(data->vehicle_list);
	while (list != NULL)
	{
	CarCost *item = list->data;
	gint dist;
	gdouble centkm;
	guint distbyvol;
	gdouble trn_amount;

		if( (item->date >= data->filter->mindate) && (item->date <= data->filter->maxdate) )
		{
			trn_amount = item->amount;

			if( item->meter == 0 )
			{
				data->total_misccost += trn_amount;
			}
			else
			{
				if(nb_refuel > 0 )
				{
					//previtem = g_list_nth_data(data->vehicle_list, nb_refuel-1);
					//if(previtem != NULL) previtem->dist = item->meter - previtem->meter;
					//DB( g_print(" + previous item dist = %d\n", item->meter - previtem->meter) );
					item->dist = item->meter - lastmeter;

					//DB( g_print(" + last meter = %d\n", lastmeter) );

				}

				lastmeter = item->meter;
				nb_refuel++;

				DB( g_print(" eval : d=%d v=%4.2f $%8.2f dist=%d\n", item->meter, item->fuel, trn_amount, item->dist) );

				//bugfix #159066
				if(item->partial == FALSE)
				{
					// full refuel after partial
					if(partial_fuel && partial_dist)
					{
						DB( g_print(" + full refuel after partial\n") );
						partial_fuel += item->fuel;
						partial_dist += item->dist;
						dist = item->dist;
						centkm = partial_dist != 0 ? partial_fuel * 100 / partial_dist : 0;
					}
					else
					{
						DB( g_print(" + real full refuel\n") );
						dist = item->dist;
						centkm = item->dist != 0 ? item->fuel * 100 / item->dist : 0;
					}
					partial_fuel = 0;
					partial_dist = 0;
				}
				// partial refuel
				else
				{
					DB( g_print(" + partial refuel\n") );
					partial_fuel += item->fuel;
					partial_dist += item->dist;
					dist = item->dist;
					centkm = 0;
				}

				distbyvol = 0;
				if(centkm != 0)
					distbyvol = hb_amount_round((1/centkm)*100, 0);

				DB( g_print(" + pf=%.2f pd=%d :: dbv=%d\n", partial_fuel, partial_dist, distbyvol) );



		    	gtk_list_store_append (GTK_LIST_STORE(model), &iter);

				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					LST_CAR_DATE    , item->date,
					LST_CAR_WORDING , item->wording,
					LST_CAR_METER   , item->meter,
					LST_CAR_FUEL    , item->fuel,
					LST_CAR_PRICE   , ABS(trn_amount) / item->fuel,
					LST_CAR_AMOUNT  , trn_amount,
					LST_CAR_DIST    , dist,
					LST_CAR_100KM   , centkm,
				    LST_CAR_DISTBYVOL, distbyvol,
				    LST_CAR_PARTIAL, item->partial,
					-1);

				DB( g_print(" + insert d=%d v=%4.2f $%8.2f %d %5.2f\n", item->meter, item->fuel, trn_amount, dist, centkm) );

				if(item->dist)
				{
					data->total_fuelcost += trn_amount;
					data->total_fuel     += item->fuel;
					data->total_dist     += item->dist;
				}


			}
		}
		list = g_list_next(list);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_report), model);
	g_object_unref(model);


	gdouble coef = data->total_dist ? 100 / (gdouble)data->total_dist : 0;

	GLOBALS->minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));


		DB( g_print(" coef = 100 / %.2f = %.2f\n", (gdouble)data->total_dist, coef) );

		// row 1 is for 100km
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][1]), "1:1");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][1]), "2:1");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][1]), "3:1");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][1]), "4:1");	//Total cost
		*/

		// 100km fuel
		buf = g_strdup_printf(PREFS->vehicle_unit_vol, data->total_fuel * coef);
		gtk_label_set_text(GTK_LABEL(data->LA_avera[CAR_RES_FUEL]), buf);
		g_free(buf);

		// 100km fuelcost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_avera[CAR_RES_FUELCOST]), data->total_fuelcost * coef, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_FUELCOST]), data->total_fuelcost * coef, GLOBALS->kcur, GLOBALS->minor);

		// 100km other cost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_avera[CAR_RES_OTHERCOST]), data->total_misccost * coef, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_OTHERCOST]), data->total_misccost * coef, GLOBALS->kcur, GLOBALS->minor);

		// 100km cost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_avera[CAR_RES_TOTALCOST]), (data->total_fuelcost + data->total_misccost) * coef, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_avera[CAR_RES_TOTALCOST]), (data->total_fuelcost + data->total_misccost) * coef, GLOBALS->kcur, GLOBALS->minor);


		// row 2 is for total
		/*
		gtk_label_set_text(GTK_LABEL(data->LA_total[1][2]), "1:2");	//Consumption
		gtk_label_set_text(GTK_LABEL(data->LA_total[2][2]), "2:2");	//Fuel cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[3][2]), "3:2");	//Other cost
		gtk_label_set_text(GTK_LABEL(data->LA_total[4][2]), "4:2");	//Total
		*/

		// total distance
		buf = g_strdup_printf(PREFS->vehicle_unit_dist, data->total_dist);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_METER]), buf);
		g_free(buf);

		// total fuel
		buf = g_strdup_printf(PREFS->vehicle_unit_vol, data->total_fuel);
		gtk_label_set_text(GTK_LABEL(data->LA_total[CAR_RES_FUEL]), buf);
		g_free(buf);

		// total fuelcost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_total[CAR_RES_FUELCOST]), data->total_fuelcost, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_FUELCOST]), data->total_fuelcost, GLOBALS->kcur, GLOBALS->minor);

		// total other cost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_total[CAR_RES_OTHERCOST]), data->total_misccost, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_OTHERCOST]), data->total_misccost, GLOBALS->kcur, GLOBALS->minor);

		// total cost
		//hb_label_set_colvaluecurr(GTK_LABEL(data->LA_total[CAR_RES_TOTALCOST]), data->total_fuelcost + data->total_misccost, GLOBALS->kcur);
		hb_label_set_colvalue(GTK_LABEL(data->LA_total[CAR_RES_TOTALCOST]), data->total_fuelcost + data->total_misccost, GLOBALS->kcur, GLOBALS->minor);


}

static void repvehicle_toggle_minor(GtkWidget *widget, gpointer user_data)
{
struct repvehicle_data *data;

	DB( g_print("(repvehicle) toggle\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	repvehicle_update(widget, NULL);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));


	/*
	statistic_update_total(widget,NULL);

	//hbfile_update(data->LV_acc, (gpointer)4);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_report));

	minor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_minor));
	gtk_chart_show_minor(GTK_CHART(data->RE_bar), minor);
	gtk_chart_show_minor(GTK_CHART(data->RE_pie), minor);
	*/

}

/*
**
*/
static void repvehicle_setup(struct repvehicle_data *data)
{
	DB( g_print("(repvehicle) setup\n") );

	data->txn_queue = g_queue_new ();
	data->vehicle_list = NULL;

	data->filter = da_filter_malloc();
	filter_default_all_set(data->filter);

	/* 3.4 : make int transfer out of stats */
	data->filter->option[FILTER_PAYMODE] = 1;
	data->filter->paymode[PAYMODE_INTXFER] = FALSE;

	filter_preset_daterange_set(data->filter, PREFS->date_range_rep, 0);
	
	g_signal_handler_block(data->PO_mindate, data->handler_id[HID_REPVEHICLE_MINDATE]);
	g_signal_handler_block(data->PO_maxdate, data->handler_id[HID_REPVEHICLE_MAXDATE]);

	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);

	g_signal_handler_unblock(data->PO_mindate, data->handler_id[HID_REPVEHICLE_MINDATE]);
	g_signal_handler_unblock(data->PO_maxdate, data->handler_id[HID_REPVEHICLE_MAXDATE]);


	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->h_cat);

	g_signal_handler_block(data->PO_cat, data->handler_id[HID_REPVEHICLE_VEHICLE]);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat), GLOBALS->vehicle_category);
	g_signal_handler_unblock(data->PO_cat, data->handler_id[HID_REPVEHICLE_VEHICLE]);



}


/*
**
*/
static gboolean repvehicle_window_dispose(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
struct repvehicle_data *data = user_data;
struct WinGeometry *wg;

	DB( g_print("(repvehicle) dispose\n") );

	g_queue_free (data->txn_queue);
	
	da_vehiclecost_destroy(data->vehicle_list);

	da_filter_free(data->filter);
	
	g_free(data);

	//store position and size
	wg = &PREFS->cst_wg;
	gtk_window_get_position(GTK_WINDOW(widget), &wg->l, &wg->t);
	gtk_window_get_size(GTK_WINDOW(widget), &wg->w, &wg->h);

	DB( g_print(" window: l=%d, t=%d, w=%d, h=%d\n", wg->l, wg->t, wg->w, wg->h) );

	//enable define windows
	GLOBALS->define_off--;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_SENSITIVE));

	return FALSE;
}


// the window creation
GtkWidget *repcost_window_new(void)
{
struct repvehicle_data *data;
struct WinGeometry *wg;
GtkWidget *window, *mainvbox, *hbox, *vbox, *treeview;
GtkWidget *label, *widget, *table;
gint row, col;
GtkUIManager *ui;
GtkActionGroup *actions;
GtkAction *action;
GError *error = NULL;

	data = g_malloc0(sizeof(struct repvehicle_data));
	if(!data) return NULL;

	DB( g_print("(repvehicle) new\n") );

	//disable define windows
	GLOBALS->define_off++;
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(2));

    /* create window, etc */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	data->window = window;

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);

	gtk_window_set_title (GTK_WINDOW (window), _("Vehicle cost report"));

	//set the window icon
	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_REP_CAR);



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
	label = make_label_widget(_("Vehi_cle:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

	widget = ui_cat_comboboxentry_new(label);
	data->PO_cat = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

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

	/* set which action should have priority in the toolbar */
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


	// total
	table = gtk_grid_new ();
	gtk_widget_set_hexpand (GTK_WIDGET(table), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_SMALL);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);

	row = 0; col = 1;

	label = make_label_widget(_("Meter:"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	col++;
	label = make_label_widget(_("Consumption:"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	col++;
	label = make_label_widget(_("Fuel cost:"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	col++;
	label = make_label_widget(_("Other cost:"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	col++;
	label = make_label_widget(_("Total cost:"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	row++;
	col = 0;
	label = make_label_widget(PREFS->vehicle_unit_100);
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	for(col = 1;col<MAX_CAR_RES;col++)
	{
		label = make_label(NULL, 1.0, 0.5);
		gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);
		data->LA_avera[col] = label;
	}

	row++;
	col = 0;
	label = make_label_widget(_("Total"));
	gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);

	for(col = 1;col<MAX_CAR_RES;col++)
	{
		label = make_label(NULL, 1.0, 0.5);
		gtk_grid_attach (GTK_GRID (table), label, col, row, 1, 1);
		data->LA_total[col] = label;
	}
	

	//detail
	widget = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	treeview = create_list_repvehicle();
	data->LV_report = treeview;
	gtk_container_add (GTK_CONTAINER(widget), treeview);
    gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);


	//todo:should move this
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_minor),GLOBALS->minor);



	/* attach our minor to treeview */
	g_object_set_data(G_OBJECT(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_report))), "minor", (gpointer)data->CM_minor);



	/* signal connect */
    g_signal_connect (window, "delete-event", G_CALLBACK (repvehicle_window_dispose), (gpointer)data);

	g_signal_connect (data->CM_minor, "toggled", G_CALLBACK (repvehicle_toggle_minor), NULL);

    data->handler_id[HID_REPVEHICLE_MINDATE] = g_signal_connect (data->PO_mindate, "changed", G_CALLBACK (repvehicle_date_change), (gpointer)data);
    data->handler_id[HID_REPVEHICLE_MAXDATE] = g_signal_connect (data->PO_maxdate, "changed", G_CALLBACK (repvehicle_date_change), (gpointer)data);

	data->handler_id[HID_REPVEHICLE_RANGE] = g_signal_connect (data->CY_range, "changed", G_CALLBACK (repvehicle_range_change), NULL);

	data->handler_id[HID_REPVEHICLE_VEHICLE] = g_signal_connect (data->PO_cat, "changed", G_CALLBACK (repvehicle_compute), NULL);


	//setup, init and show window
	repvehicle_setup(data);

	/* toolbar */
	/*
	if(PREFS->toolbar_style == 0)
		gtk_toolbar_unset_style(GTK_TOOLBAR(data->TB_bar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(data->TB_bar), PREFS->toolbar_style-1);
	*/

	//setup, init and show window
	wg = &PREFS->cst_wg;
	gtk_window_move(GTK_WINDOW(window), wg->l, wg->t);
	gtk_window_resize(GTK_WINDOW(window), wg->w, wg->h);

	gtk_widget_show_all (window);

	//minor ?
	if( PREFS->euro_active )
		gtk_widget_show(data->CM_minor);
	else
		gtk_widget_hide(data->CM_minor);



	if( PREFS->date_range_rep )
		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_range), PREFS->date_range_rep);
	else
		repvehicle_compute(window, NULL);

	return(window);
}

/*
** ============================================================================
*/

static void repvehicle_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
GDate *date;
guint32 julian;
gchar buf[256];

	gtk_tree_model_get(model, iter,
		LST_CAR_DATE, &julian,
		-1);

	date = g_date_new_julian (julian);
	g_date_strftime (buf, 256-1, PREFS->date_format, date);
	g_date_free(date);

	g_object_set(renderer, "text", buf, NULL);
}

static void repvehicle_distance_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
guint distance;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &distance, -1);

	if(distance != 0)
	{
		text = g_strdup_printf(PREFS->vehicle_unit_dist, distance);
		g_object_set(renderer, "text", text, NULL);
		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

static void repvehicle_volume_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
gdouble volume;
gboolean partial;
gchar *text;

	gtk_tree_model_get(model, iter, user_data, &volume, LST_CAR_PARTIAL, &partial, -1);

	if(volume != 0)
	{
		text = g_strdup_printf(PREFS->vehicle_unit_vol, volume);
		g_object_set(renderer, 
			"text", text,
		    "style-set", TRUE,
			"style", partial ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL,   
		    NULL);

		g_free(text);
	}
	else
		g_object_set(renderer, "text", "-", NULL);
}

static void repvehicle_amount_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gdouble  value;
gchar *color;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

	gtk_tree_model_get(model, iter,
		user_data, &value,
		-1);

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

static GtkTreeViewColumn *volume_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_volume_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *distance_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_distance_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}

static GtkTreeViewColumn *amount_list_repvehicle_column(gchar *name, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, name);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_sort_column_id (column, id);
	return column;
}


/*
** create our statistic list
*/
static GtkWidget *create_list_repvehicle(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_CAR,
		G_TYPE_UINT,
		G_TYPE_STRING,
		G_TYPE_UINT,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_DOUBLE,
		G_TYPE_UINT,
		G_TYPE_DOUBLE,
	    G_TYPE_UINT,
	    G_TYPE_BOOLEAN		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);

	/* column date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", LST_CAR_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_date_cell_data_function, NULL, NULL);

/*
	LST_CAR_DATE,
	LST_CAR_WORDING,
	LST_CAR_METER,
	LST_CAR_FUEL,
	LST_CAR_PRICE,
	LST_CAR_AMOUNT,
	LST_CAR_DIST,
	LST_CAR_100KM

*/

	/* column: Wording */
/*
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Wording"));
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", LST_CAR_WORDING);
	//gtk_tree_view_column_set_cell_data_func(column, renderer, repvehicle_text_cell_data_function, NULL, NULL);
*/

	/* column: Meter */
	column = distance_list_repvehicle_column(_("Meter"), LST_CAR_METER);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Fuel load */
	column = volume_list_repvehicle_column(_("Fuel"), LST_CAR_FUEL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Price by unit */
	column = amount_list_repvehicle_column(_("Price"), LST_CAR_PRICE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = amount_list_repvehicle_column(_("Amount"), LST_CAR_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Distance done */
	column = distance_list_repvehicle_column(_("Dist."), LST_CAR_DIST);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: consumption for 100Km */
	column = volume_list_repvehicle_column(PREFS->vehicle_unit_100, LST_CAR_100KM);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: km by liter (distance by volume */
	column = distance_list_repvehicle_column(PREFS->vehicle_unit_distbyvol, LST_CAR_DISTBYVOL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


  /* column last: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}
