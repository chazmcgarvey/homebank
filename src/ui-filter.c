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

#include "ui-filter.h"
#include "ui-account.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "gtk-dateentry.h"


/****************************************************************************/
/* Debug macros										 */
/****************************************************************************/
#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;


extern char *paymode_label_names[];


gchar *CYA_FLT_TYPE[] = {
	N_("Expense"),
	N_("Income"),
	"",
	N_("Any Type"),
	NULL
};

gchar *CYA_FLT_STATUS[] = {
	N_("Uncategorized"),
	N_("Unreconciled"),
	N_("Uncleared"),
	N_("Reconciled"),
	N_("Cleared"),
	"",
	N_("Any Status"),
	NULL
};

gchar *CYA_FLT_RANGE[] = {
	N_("This month"),
	N_("Last month"),
	N_("This quarter"),
	N_("Last quarter"),
	N_("This year"),
	N_("Last year"),
	"",
	N_("Last 30 days"),
	N_("Last 60 days"),
	N_("Last 90 days"),
	N_("Last 12 months"),
	"",
	N_("Other..."),
	"",
	N_("All date"),
	NULL
};


gchar *CYA_SELECT[] =
{
	"----",
	N_("All month"),
	N_("January"),
	N_("February"),
	N_("March"),
	N_("April"),
	N_("May"),
	N_("June"),
	N_("July"),
	N_("August"),
	N_("September"),
	N_("October"),
	N_("November"),
	N_("December"),
	NULL
};


/* = = = = = = = = = = = = = = = = = = = = */

/*
**
*/
static void ui_flt_manage_acc_select(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean toggle;

	DB( g_print("(ui_flt_manage) acc select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFACC_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, toggle, -1);
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}

/*
**
*/
static void ui_flt_manage_pay_select(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
gboolean toggle;

	DB( g_print("(ui_flt_manage) pay select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFPAY_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, toggle, -1);
				break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
}

/*
**
*/
static void ui_flt_manage_cat_select(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;
gint select = GPOINTER_TO_INT(user_data);
GtkTreeModel *model;
GtkTreeIter	iter, child;
gboolean valid;
gint n_child;
gboolean toggle;

	DB( g_print("(ui_flt_manage) pay select\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		switch(select)
		{
			case BUTTON_ALL:
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, TRUE, -1);
				break;
			case BUTTON_NONE:
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, FALSE, -1);
				break;
			case BUTTON_INVERT:
					gtk_tree_model_get (model, &iter, LST_DEFCAT_TOGGLE, &toggle, -1);
					toggle ^= 1;
					gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, toggle, -1);
				break;
		}

		n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
		gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
		while(n_child > 0)
		{

			switch(select)
			{
				case BUTTON_ALL:
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, TRUE, -1);
					break;
				case BUTTON_NONE:
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, FALSE, -1);
					break;
				case BUTTON_INVERT:
						gtk_tree_model_get (model, &child, LST_DEFCAT_TOGGLE, &toggle, -1);
						toggle ^= 1;
						gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, toggle, -1);
					break;
			}

			n_child--;
			gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
		}

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

}

/*
**
*/
static void ui_flt_manage_option_update(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;
gint active, i;
gboolean sensitive;

	DB( g_print("(ui_flt_manage) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// status
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_STATUS]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_reconciled, sensitive);
	gtk_widget_set_sensitive(data->CM_cleared, sensitive);

	// date
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_DATE]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->PO_mindate, sensitive);
	gtk_widget_set_sensitive(data->PO_maxdate, sensitive);
	gtk_widget_set_sensitive(data->CY_month, sensitive);
	gtk_widget_set_sensitive(data->NB_year, sensitive);

	// amount
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_AMOUNT]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->ST_minamount, sensitive);
	gtk_widget_set_sensitive(data->ST_maxamount, sensitive);

	// text
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_TEXT]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_exact, sensitive);
	gtk_widget_set_sensitive(data->ST_wording, sensitive);
	gtk_widget_set_sensitive(data->ST_info, sensitive);
	gtk_widget_set_sensitive(data->ST_tag, sensitive);

	//paymode
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_PAYMODE]));
	sensitive = active == 0 ? FALSE : TRUE;
	for(i=0;i<NUM_PAYMODE_MAX;i++)
	{
		gtk_widget_set_sensitive(data->CM_paymode[i], sensitive);
	}

	//account
	if(data->show_account == TRUE)
	{
		active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_ACCOUNT]));
		sensitive = active == 0 ? FALSE : TRUE;
		gtk_widget_set_sensitive(data->LV_acc, sensitive);
		for(i=0;i<MAX_BUTTON;i++)
		{
			gtk_widget_set_sensitive(data->BT_acc[i], sensitive);
		}


	}

	//payee
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_PAYEE]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->LV_pay, sensitive);
	for(i=0;i<MAX_BUTTON;i++)
	{
		gtk_widget_set_sensitive(data->BT_pay[i], sensitive);
	}

	//category
	active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[FILTER_CATEGORY]));
	sensitive = active == 0 ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->LV_cat, sensitive);
	for(i=0;i<MAX_BUTTON;i++)
	{
		gtk_widget_set_sensitive(data->BT_cat[i], sensitive);
	}



}


/*
**
*/
static void ui_flt_manage_get(struct ui_flt_manage_data *data)
{
gint i;
gchar *txt;

	DB( g_print("(ui_flt_manage) get\n") );

	if(data->filter !=NULL)
	{
	GtkTreeModel *model;
	//GtkTreeSelection *selection;
	GtkTreeIter	iter, child;
	gint n_child;
	gboolean valid;
	gboolean toggled;

		for(i=0;i<FILTER_MAX;i++)
		{
			if(data->show_account == FALSE && i == FILTER_ACCOUNT)
				continue;

			data->filter->option[i] = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_option[i]));
		}

	//date
		DB( g_print(" date\n") );
		data->filter->mindate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_mindate));
		data->filter->maxdate = gtk_date_entry_get_date(GTK_DATE_ENTRY(data->PO_maxdate));

	//status
		DB( g_print(" status\n") );
		data->filter->reconciled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_reconciled));
		data->filter->cleared  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cleared));

		data->filter->forceadd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forceadd));
		data->filter->forcechg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forcechg));
		data->filter->forceremind  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_forceremind));

	//paymode
		DB( g_print(" paymode\n") );
		for(i=0;i<NUM_PAYMODE_MAX;i++)
			data->filter->paymode[i] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]));

	//amount
		data->filter->minamount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minamount));
		data->filter->maxamount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_maxamount));

	//text:wording
		data->filter->exact  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_exact));
		//free any previous string
		if(	data->filter->wording )
		{
			g_free(data->filter->wording);
			data->filter->wording = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_wording));

		if (txt && *txt)	// ignore if entry is empty
		{
			data->filter->wording = g_strdup(txt);
		}

	//text:info
		//free any previous string
		if(	data->filter->info )
		{
			g_free(data->filter->info);
			data->filter->info = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
		// ignore if entry is empty
		if (txt && *txt)
		{
			data->filter->info = g_strdup(txt);
		}

	//text:tag
		//free any previous string
		if(	data->filter->tag )
		{
			g_free(data->filter->tag);
			data->filter->tag = NULL;
		}
		txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tag));
		// ignore if entry is empty
		if (txt && *txt)
		{
			data->filter->tag = g_strdup(txt);
		}


	// account
		if(data->show_account == TRUE)
		{
			DB( g_print(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Account *accitem;

				gtk_tree_model_get (model, &iter,
					LST_DEFACC_TOGGLE, &toggled,
					LST_DEFACC_DATAS, &accitem,
					-1);

				//data->filter->acc[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
				accitem->filter = toggled;

				/* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}
		}

	// payee
		DB( g_print(" payee\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Payee *payitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFPAY_TOGGLE, &toggled,
				LST_DEFPAY_DATAS, &payitem,
				-1);

			//data->filter->pay[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			payitem->filter = toggled;

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// category
		DB( g_print(" category\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
		//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Category *catitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFCAT_TOGGLE, &toggled,
				LST_DEFCAT_DATAS, &catitem,
				-1);

			//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &iter);
			//data->filter->cat[i] = toggled;
			catitem->filter = toggled;

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get (model, &child,
					LST_DEFCAT_TOGGLE, &toggled,
					LST_DEFCAT_DATAS, &catitem,
					-1);


				//data->filter->cat[i] = toggled;
				//data->filter->cat[i] = gtk_tree_selection_iter_is_selected(selection, &child);
				catitem->filter = toggled;

				n_child--;
				gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
			}

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// active tab
	data->filter->last_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(data->notebook));
	DB( g_print(" page is %d\n", data->filter->last_tab) );
	

	}
}


/*
**
*/
static void ui_flt_manage_set(struct ui_flt_manage_data *data)
{

	DB( g_print("(ui_flt_manage) set\n") );

	if(data->filter != NULL)
	{
	GtkTreeModel *model;
	//GtkTreeSelection *selection;
	GtkTreeIter	iter, child;
	GDate *date;
	gint n_child;
	gboolean valid;
	gint i;

		DB( g_print(" options\n") );

		for(i=0;i<FILTER_MAX;i++)
		{
			if(data->show_account == FALSE && i == FILTER_ACCOUNT)
				continue;

			gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_option[i]), data->filter->option[i]);
		}

		//DB( g_print(" setdate %d to %x\n", data->filter->mindate, data->PO_mindate) );
		//DB( g_print(" setdate %d to %x\n", 0, data->PO_mindate) );
	//date
		DB( g_print(" date\n") );
		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
		gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
		date = g_date_new_julian(data->filter->maxdate);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_year), g_date_get_year(date));
		g_date_free(date);

	//status
		DB( g_print(" status\n") );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_reconciled), data->filter->reconciled);


		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forceadd), data->filter->forceadd);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forcechg), data->filter->forcechg);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_forceremind), data->filter->forceremind);

	//paymode
		DB( g_print(" paymode\n") );

		for(i=0;i<NUM_PAYMODE_MAX;i++)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_paymode[i]), data->filter->paymode[i]);

	//amount
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_minamount), data->filter->minamount);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_maxamount), data->filter->maxamount);

	//text
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_exact), data->filter->exact);
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), (data->filter->info != NULL) ? data->filter->info : "");
	gtk_entry_set_text(GTK_ENTRY(data->ST_wording), (data->filter->wording != NULL) ? data->filter->wording : "");
	gtk_entry_set_text(GTK_ENTRY(data->ST_tag), (data->filter->tag != NULL) ? data->filter->tag : "");

	//account
		if(data->show_account == TRUE)
		{
			DB( g_print(" account\n") );

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
			//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
			i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
			while (valid)
			{
			Account *accitem;

				gtk_tree_model_get (model, &iter,
					LST_DEFACC_DATAS, &accitem,
					-1);

				if(accitem->filter == TRUE)
					//gtk_tree_selection_select_iter(selection, &iter);
					gtk_list_store_set (GTK_LIST_STORE (model), &iter,
						LST_DEFACC_TOGGLE, TRUE, -1);

				/* Make iter point to the next row in the list store */
				i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
			}
		}

	// payee
		DB( g_print(" payee\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Payee *payitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFPAY_DATAS, &payitem,
				-1);

			if(payitem->filter == TRUE)
				gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, TRUE, -1);

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// category
		DB( g_print(" category\n") );

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
		//selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
		i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Category *catitem;

			gtk_tree_model_get (model, &iter,
				LST_DEFCAT_DATAS, &catitem,
				-1);

			if(catitem->filter == TRUE)
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, TRUE, -1);

			n_child = gtk_tree_model_iter_n_children (GTK_TREE_MODEL(model), &iter);
			gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
			while(n_child > 0)
			{
				i++;

				gtk_tree_model_get (model, &child,
					LST_DEFCAT_DATAS, &catitem,
					-1);

				if(catitem->filter == TRUE)
					gtk_tree_store_set (GTK_TREE_STORE (model), &child, LST_DEFCAT_TOGGLE, TRUE, -1);

				n_child--;
				gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
			}

			/* Make iter point to the next row in the list store */
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	// active tab
	gtk_notebook_set_current_page(GTK_NOTEBOOK(data->notebook), data->filter->last_tab);
	DB( g_print(" set page %d\n", data->filter->last_tab) );

	}
}


/*
**
*/
static void ui_flt_manage_clear(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_flt_manage) clear\n") );

	filter_default_all_set(data->filter);

	ui_flt_manage_set(data);

}


/*
**
*/
static void ui_flt_manage_setup(struct ui_flt_manage_data *data)
{

	DB( g_print("(ui_flt_manage) setup\n") );

	if(data->show_account == TRUE && data->LV_acc != NULL)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc))), GTK_SELECTION_MULTIPLE);

		ui_acc_listview_populate(data->LV_acc, ACC_LST_INSERT_REPORT);
		//populate_view_acc(data->LV_acc, GLOBALS->acc_list, FALSE);
	}

	if(data->LV_pay)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay))), GTK_SELECTION_MULTIPLE);

		ui_pay_listview_populate(data->LV_pay);
		//populate_view_pay(data->LV_pay, GLOBALS->pay_list, FALSE);
	}

	if(data->LV_cat)
	{
		//gtk_tree_selection_set_mode(GTK_TREE_SELECTION(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat))), GTK_SELECTION_MULTIPLE);

		//populate_view_cat(data->LV_cat, GLOBALS->cat_list, FALSE);
		ui_cat_listview_populate(data->LV_cat, CAT_TYPE_ALL);
		gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
	}
}

/*
**
*/
static GtkWidget *ui_flt_manage_page_category (struct ui_flt_manage_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (container), SPACING_MEDIUM);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label_widget(_("_Option:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_CATEGORY] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_CATEGORY], TRUE, TRUE, 0);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), SPACING_SMALL);

	data->LV_cat = (GtkWidget *)ui_cat_listview_new(TRUE, FALSE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_cat);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_cat[BUTTON_INVERT] = widget;

	return(container);
}

/*
**
*/
static GtkWidget *ui_flt_manage_page_payee (struct ui_flt_manage_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (container), SPACING_MEDIUM);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label_widget(_("_Option:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_PAYEE] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_PAYEE], TRUE, TRUE, 0);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), SPACING_SMALL);

	data->LV_pay = (GtkWidget *)ui_pay_listview_new(TRUE, FALSE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_pay);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_pay[BUTTON_INVERT] = widget;

	return(container);
}

/*
** account filter
*/
static GtkWidget *ui_flt_manage_page_account (struct ui_flt_manage_data *data)
{
GtkWidget *container, *scrollwin, *hbox, *vbox, *label, *widget;

	container = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (container), SPACING_MEDIUM);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, FALSE, FALSE, 0);

	label = make_label_widget(_("_Option:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	data->CY_option[FILTER_ACCOUNT] = make_nainex(label);
	gtk_box_pack_start (GTK_BOX (hbox), data->CY_option[FILTER_ACCOUNT], TRUE, TRUE, 0);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (container), hbox, TRUE, TRUE, 0);

 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start (GTK_BOX (hbox), scrollwin, TRUE, TRUE, 0);

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), SPACING_SMALL);

	data->LV_acc = ui_acc_listview_new(TRUE);
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_acc);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	widget = gtk_button_new_with_label(_("All"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_ALL] = widget;

	widget = gtk_button_new_with_label(_("None"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_NONE] = widget;

	widget = gtk_button_new_with_label(_("Invert"));
	gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 0);
	data->BT_acc[BUTTON_INVERT] = widget;

	return(container);
}


static void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate)
{
GDate *date;

	date = g_date_new();
	if(month)
	{
		// mindate is month 1, year :: maxdate is last day of month, year
		g_date_set_dmy(date, 1, month, year);
		*mindate = g_date_get_julian(date);
		g_date_add_days(date, g_date_get_days_in_month(month, year));
		*maxdate = g_date_get_julian(date)-1;
	}
	else
	{
		g_date_set_dmy(date, 1, 1, year);
		*mindate = g_date_get_julian(date);
		g_date_set_dmy(date, 31, 12, year);
		*maxdate = g_date_get_julian(date);
	}
	g_date_free(date);
}



static void ui_flt_manage_period_change(GtkWidget *widget, gpointer user_data)
{
struct ui_flt_manage_data *data;
gint month, year;

	DB( g_print("(ui_flt_manage) period change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");


	month = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_month));
	year = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_year));

	DB( g_print(" month=%d, year=%d\n", month, year) );


	if(month != 0)
		get_period_minmax(month-1, year, &data->filter->mindate, &data->filter->maxdate);
	else
		get_period_minmax(0, year, &data->filter->mindate, &data->filter->maxdate);

	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_mindate), data->filter->mindate);
	gtk_date_entry_set_date(GTK_DATE_ENTRY(data->PO_maxdate), data->filter->maxdate);
}



static GtkWidget *ui_flt_manage_part_date(struct ui_flt_manage_data *data)
{
GtkWidget *table, *label;
gint row;

	// filter date
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_container_set_border_width(GTK_CONTAINER(table), SPACING_MEDIUM);

	row = 0;
	label = make_label_group(_("Filter Date"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

		row++;
		label = make_label_widget(_("_Option:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->CY_option[FILTER_DATE] = make_nainex(label);
		gtk_grid_attach (GTK_GRID (table), data->CY_option[FILTER_DATE], 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_From:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->PO_mindate = gtk_date_entry_new();
		//data->PO_mindate = gtk_entry_new();
		//gtk_grid_attach (GTK_GRID (table), data->PO_mindate, 1, 2, row, row+1);
		gtk_grid_attach (GTK_GRID (table), data->PO_mindate, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_To:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->PO_maxdate = gtk_date_entry_new();
		//data->PO_maxdate = gtk_entry_new();
		//gtk_grid_attach (GTK_GRID (table), data->PO_maxdate, 1, 2, row, row+1);
		gtk_grid_attach (GTK_GRID (table), data->PO_maxdate, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_Month:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->CY_month = make_cycle(label, CYA_SELECT);
		gtk_grid_attach (GTK_GRID (table), data->CY_month, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_Year:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->NB_year = make_year(label);
		gtk_grid_attach (GTK_GRID (table), data->NB_year, 2, row, 1, 1);


	return table;
}


static GtkWidget *ui_flt_manage_part_text(struct ui_flt_manage_data *data)
{
GtkWidget *table, *label;
gint row;

	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_container_set_border_width(GTK_CONTAINER(table), SPACING_MEDIUM);

	row = 0;
	label = make_label_group(_("Filter Text"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

		row++;
		label = make_label_widget(_("_Option:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

		//gtk_grid_attach (GTK_GRID (table), label, 0, 1, row, row+1);
		data->CY_option[FILTER_TEXT] = make_nainex(label);
		gtk_grid_attach (GTK_GRID (table), data->CY_option[FILTER_TEXT], 2, row, 1, 1);

		row++;
		data->CM_exact = gtk_check_button_new_with_mnemonic (_("Case _sensitive"));
		gtk_grid_attach (GTK_GRID (table), data->CM_exact, 2, row, 1, 1);
	
		row++;
		label = make_label_widget(_("_Memo:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->ST_wording = make_string(label);
		gtk_widget_set_hexpand (data->ST_wording, TRUE);
		gtk_grid_attach (GTK_GRID (table), data->ST_wording, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_Info:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->ST_info = make_string(label);
		gtk_widget_set_hexpand (data->ST_info, TRUE);
		gtk_grid_attach (GTK_GRID (table), data->ST_info, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_Tag:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->ST_tag = make_string(label);
		gtk_widget_set_hexpand (data->ST_tag, TRUE);
		gtk_grid_attach (GTK_GRID (table), data->ST_tag, 2, row, 1, 1);


	return table;
}

static GtkWidget *ui_flt_manage_part_amount(struct ui_flt_manage_data *data)
{
GtkWidget *table, *label;
gint row;


	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_container_set_border_width(GTK_CONTAINER(table), SPACING_MEDIUM);

	// Amount section
	row = 0;

	label = make_label_group(_("Filter Amount"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

		row++;
		label = make_label_widget(_("_Option:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

		//gtk_grid_attach (GTK_GRID (table), label, 0, 1, row, row+1);
		data->CY_option[FILTER_AMOUNT] = make_nainex(label);
		gtk_grid_attach (GTK_GRID (table), data->CY_option[FILTER_AMOUNT], 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_From:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->ST_minamount = make_amount(label);
		gtk_grid_attach (GTK_GRID (table), data->ST_minamount, 2, row, 1, 1);

		row++;
		label = make_label_widget(_("_To:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->ST_maxamount = make_amount(label);
		gtk_grid_attach (GTK_GRID (table), data->ST_maxamount, 2, row, 1, 1);



	return table;
}


static GtkWidget *ui_flt_manage_part_status(struct ui_flt_manage_data *data)
{
GtkWidget *table, *label, *vbox, *widget;
gint row;


		// column 2

	// filter status
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_container_set_border_width(GTK_CONTAINER(table), SPACING_MEDIUM);

	row = 0;
	label = make_label_group (_("Filter Status"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

		row++;
		label = make_label_widget(_("_Option:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

		data->CY_option[FILTER_STATUS] = make_nainex(label);
		gtk_grid_attach (GTK_GRID (table), data->CY_option[FILTER_STATUS], 2, row, 1, 1);

		row++;
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_grid_attach (GTK_GRID (table), vbox, 2, row, 1, 1);

		widget = gtk_check_button_new_with_mnemonic (_("reconciled"));
		data->CM_reconciled = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		widget = gtk_check_button_new_with_mnemonic (_("cleared"));
		data->CM_cleared = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		row++;
		label = make_label_widget(_("Force:"));
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_grid_attach (GTK_GRID (table), vbox, 2, row, 1, 1);

		widget = gtk_check_button_new_with_mnemonic (_("display 'Added'"));
		data->CM_forceadd = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		widget = gtk_check_button_new_with_mnemonic (_("display 'Edited'"));
		data->CM_forcechg = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

		widget = gtk_check_button_new_with_mnemonic (_("display 'Remind'"));
		data->CM_forceremind = widget;
		gtk_box_pack_start (GTK_BOX (vbox), widget, TRUE, TRUE, 0);

	return table;
}


static GtkWidget *ui_flt_manage_part_paymode(struct ui_flt_manage_data *data)
{
GtkWidget *table, *label, *table1, *image;
gint i, row;

	// Filter Payment
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_container_set_border_width(GTK_CONTAINER(table), SPACING_MEDIUM);


	row = 0;
	label = make_label_group(_("Filter Payment"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);


		row++;
		label = make_label_widget(_("_Option:"));
		//----------------------------------------- l, r, t, b
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		data->CY_option[FILTER_PAYMODE] = make_nainex(label);
		gtk_grid_attach (GTK_GRID (table), data->CY_option[FILTER_PAYMODE], 2, row, 1, 1);

		table1 = gtk_grid_new ();
		gtk_grid_set_row_spacing (GTK_GRID (table1), 0);
		gtk_grid_set_column_spacing (GTK_GRID (table1), 2);

		row++;
		gtk_grid_attach (GTK_GRID (table), table1, 2, row, 1, 1);

		for(i=0;i<NUM_PAYMODE_MAX;i++)
		{
			row = i;

			data->CM_paymode[i] = gtk_check_button_new();
			gtk_grid_attach (GTK_GRID (table1), data->CM_paymode[i], 0, row, 1, 1);

			image = gtk_image_new_from_icon_name( get_paymode_icon_name(i), GTK_ICON_SIZE_MENU);
			gtk_grid_attach (GTK_GRID (table1), image, 1, row, 1, 1);

			label = make_label(_(paymode_label_names[i]), 0.0, 0.5);
			gtk_grid_attach (GTK_GRID (table1), label, 2, row, 1, 1);

		}



	return table;
}


/*
** general page: date, amount, status, payment
*/
/*
static GtkWidget *ui_flt_manage_page_general (struct ui_flt_manage_data *data)
{
GtkWidget *container, *part;

	//container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	//gtk_container_set_border_width(GTK_CONTAINER(container), SPACING_SMALL);

	container = gtk_grid_new (2, 3, FALSE);
	gtk_grid_set_row_spacing (GTK_GRID (container), SPACING_SMALL*2);
	gtk_grid_set_column_spacing (GTK_GRID (container), SPACING_MEDIUM*2);
	gtk_container_set_border_width(GTK_CONTAINER(container), SPACING_SMALL);

	// date: r=1, c=1
	part = ui_flt_manage_part_date(data);
	gtk_grid_attach(GTK_GRID (container), part, 0, 1, 0, 1);

	// amount: r=2, c=2
	part = ui_flt_manage_part_amount(data);
	gtk_grid_attach (GTK_GRID (container), part, 0, 1, 1, 2);

	// paymode:
	part = ui_flt_manage_part_paymode(data);
	gtk_grid_attach (GTK_GRID (container), part, 1, 2, 0, 2);

	// status: r=2, c=1
	part = ui_flt_manage_part_status(data);
	gtk_grid_attach (GTK_GRID (container), part, 2, 3, 0, 1);

	// text: r=2, c=1
	part = ui_flt_manage_part_text(data);
	gtk_grid_attach (GTK_GRID (container), part, 2, 3, 1, 2);

	gtk_container_set_border_width(GTK_CONTAINER(container), SPACING_SMALL);

	return(container);
}
*/



/*
**
*/
gint ui_flt_manage_dialog_new(GtkWidget *widget, Filter *filter, gboolean show_account)
{
struct ui_flt_manage_data data;
GtkWidget *parentwindow, *window, *content, *mainbox, *notebook, *label, *page;

	//data = g_malloc0(sizeof(struct ui_flt_manage_data));
	//if(!data) return NULL;
	memset(&data, 0, sizeof(data));

	data.filter = filter;

	parentwindow = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW);

	window = gtk_dialog_new_with_buttons (_("Edit filter"),
					    GTK_WINDOW (parentwindow),
					    0,
					    _("_Reset"),
					    55,
					    _("_Cancel"),
					    GTK_RESPONSE_REJECT,
					    _("_OK"),
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_FILTER);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(ui_flt_manage) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (content), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), SPACING_MEDIUM);


	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
    gtk_box_pack_start (GTK_BOX (mainbox), notebook, TRUE, TRUE, 0);
	data.notebook = notebook;

	//common (date + status + amount)
/*	label = gtk_label_new(_("General"));
	page = ui_flt_manage_page_general(&data);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
*/

	label = gtk_label_new(_("Date"));
	page = ui_flt_manage_part_date(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	label = gtk_label_new(_("Status"));
	page = ui_flt_manage_part_status(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	label = gtk_label_new(_("Payment"));
	page = ui_flt_manage_part_paymode(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	label = gtk_label_new(_("Amount"));
	page = ui_flt_manage_part_amount(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	label = gtk_label_new(_("Text"));
	page = ui_flt_manage_part_text(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
	
	label = gtk_label_new(_("Category"));
	page = ui_flt_manage_page_category(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	label = gtk_label_new(_("Payee"));
	page = ui_flt_manage_page_payee(&data);
	gtk_widget_show(GTK_WIDGET(page));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

	data.show_account = show_account;
	if(show_account == TRUE)
	{
		label = gtk_label_new(_("Account"));
		page = ui_flt_manage_page_account(&data);
		gtk_widget_show(GTK_WIDGET(page));
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);
	}

	//setup, init and show window
	ui_flt_manage_setup(&data);
	ui_flt_manage_set(&data);

	ui_flt_manage_option_update(window, NULL);

	
	/* signal connect */
    g_signal_connect (data.CY_option[FILTER_STATUS]  , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_DATE]    , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_AMOUNT]  , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_PAYMODE] , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);

    g_signal_connect (data.CY_option[FILTER_PAYEE]   , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_CATEGORY], "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);
    g_signal_connect (data.CY_option[FILTER_TEXT]    , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);

	if(show_account == TRUE)
	{
	    g_signal_connect (data.CY_option[FILTER_ACCOUNT] , "changed", G_CALLBACK (ui_flt_manage_option_update), NULL);

	    g_signal_connect (data.BT_acc[BUTTON_ALL]   , "clicked", G_CALLBACK (ui_flt_manage_acc_select), GINT_TO_POINTER(BUTTON_ALL));
	    g_signal_connect (data.BT_acc[BUTTON_NONE]  , "clicked", G_CALLBACK (ui_flt_manage_acc_select), GINT_TO_POINTER(BUTTON_NONE));
	    g_signal_connect (data.BT_acc[BUTTON_INVERT], "clicked", G_CALLBACK (ui_flt_manage_acc_select), GINT_TO_POINTER(BUTTON_INVERT));
	}

	g_signal_connect (data.CY_month, "changed", G_CALLBACK (ui_flt_manage_period_change), NULL);
	g_signal_connect (data.NB_year, "value-changed", G_CALLBACK (ui_flt_manage_period_change), NULL);

    g_signal_connect (data.BT_pay[BUTTON_ALL]   , "clicked", G_CALLBACK (ui_flt_manage_pay_select), GINT_TO_POINTER(BUTTON_ALL));
    g_signal_connect (data.BT_pay[BUTTON_NONE]  , "clicked", G_CALLBACK (ui_flt_manage_pay_select), GINT_TO_POINTER(BUTTON_NONE));
    g_signal_connect (data.BT_pay[BUTTON_INVERT], "clicked", G_CALLBACK (ui_flt_manage_pay_select), GINT_TO_POINTER(BUTTON_INVERT));

    g_signal_connect (data.BT_cat[BUTTON_ALL]   , "clicked", G_CALLBACK (ui_flt_manage_cat_select), GINT_TO_POINTER(BUTTON_ALL));
    g_signal_connect (data.BT_cat[BUTTON_NONE]  , "clicked", G_CALLBACK (ui_flt_manage_cat_select), GINT_TO_POINTER(BUTTON_NONE));
    g_signal_connect (data.BT_cat[BUTTON_INVERT], "clicked", G_CALLBACK (ui_flt_manage_cat_select), GINT_TO_POINTER(BUTTON_INVERT));


	gtk_widget_show_all (window);


	//wait for the user
	gint retval;	// = 55;

	//while( result == 55 )
	//{
		retval = gtk_dialog_run (GTK_DIALOG (window));

		switch (retval)
	    {
		case GTK_RESPONSE_ACCEPT:
		   //do_application_specific_something ();
		   ui_flt_manage_get(&data);
		   break;
		case 55:
			ui_flt_manage_clear(window, NULL);
		   ui_flt_manage_get(&data);
			break;
		default:
		   //do_nothing_since_dialog_was_cancelled ();
		   break;
	    }
	//}

	// cleanup and destroy
	//ui_flt_manage_cleanup(&data, result);


	DB( g_print(" free\n") );
	//g_free(data);

	DB( g_print(" destroy\n") );
	gtk_widget_destroy (window);

	DB( g_print(" all ok\n") );

	return retval;
}
