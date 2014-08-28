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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty ofdeftransaction_amountchanged
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-transaction.h"
#include "hb-transaction.h"
#include "gtk-dateentry.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-account.h"


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


extern gchar *CYA_TYPE[];

gchar *CYA_OPERATION[] = {
	N_("Add transaction"),
	N_("Inherit transaction"),
	N_("Modify transaction")
};

static void deftransaction_update(GtkWidget *widget, gpointer user_data);

#define GTK_RESPONSE_SPLIT_SUM 10880
#define GTK_RESPONSE_SPLIT_REM 10888

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void ui_txn_split_dialog_line_sensitive(guint line, gboolean sensitive, gpointer user_data)
{
struct ui_txn_split_dialog_data *data = user_data;

	if( line > TXN_MAX_SPLIT )
		return;

	if( line == 0 ) // line 0 always active !
		sensitive = TRUE;

	
	gtk_widget_set_sensitive(data->PO_cat[line], sensitive);
	gtk_widget_set_sensitive(data->ST_amount[line], sensitive);
	gtk_widget_set_sensitive(data->ST_memo[line], sensitive);
	if(data->BT_rem[line])
		gtk_widget_set_sensitive(data->BT_rem[line], sensitive);
	if(data->BT_add[line])
		gtk_widget_set_sensitive(data->BT_add[line], sensitive);
	
	if(sensitive == FALSE)
	{
		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat[line]), 0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount[line]), 0.0);
		gtk_entry_set_text(GTK_ENTRY(data->ST_memo[line]), "");
	}

	if(sensitive == TRUE)
		data->activeline = line;

}


static void ui_txn_split_dialog_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_txn_split_dialog_data *data = user_data;
gint i, count, nbvalid;
//gint j;
gchar buf[48];
gboolean sensitive, active;
//guint32 cat[TXN_MAX_SPLIT];
gdouble amt[TXN_MAX_SPLIT];
gboolean valid[TXN_MAX_SPLIT];

	DB( g_print("\n(ui_txn_split_dialog_compute)\n") );

	data->sumsplit = data->remsplit = 0.0;
	nbvalid = 0;
	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		active = gtk_widget_get_sensitive(data->PO_cat[i]);
		if(!active) break;
		
		//cat[i] = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_cat[i]));
		amt[i] = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount[i]));
		data->sumsplit += amt[i];
		valid[i] = TRUE;

		if(!amt[i])
			valid[i] = FALSE;

		/* disable use same category several time
		for(j=0;j<i;j++)
		{
			if(i == j) continue;
			if( (cat[i] == cat[j]) )
			{
				valid[i] = FALSE;
				break;
			}
		}*/
		DB( g_print("- split %d : act.=%d val.=%d : amt=%.2f\n", i, active, valid[i], amt[i]) );

		if(valid[i])
			nbvalid++;

		DB( g_print("- nbsplit %d\n", data->nbsplit) );

		if(data->nbsplit == i)
		{
			DB( g_print("- set last split %d\n", i) );

			if(data->BT_add[i])
				gtk_widget_set_sensitive(data->BT_add[i], valid[i]);

			if(data->BT_rem[i])
				gtk_widget_set_sensitive(data->BT_rem[i], TRUE);
		}
		else
		{
			DB( g_print("- set off to %d\n", i) );

			if(data->BT_add[i])
				gtk_widget_set_sensitive(data->BT_add[i], FALSE);

			if(data->BT_rem[i])
				gtk_widget_set_sensitive(data->BT_rem[i], FALSE);
		}
	}

	count = i;
	DB( g_print("- count=%d, nbvalid=%d\n", count, nbvalid ) );

	
	if(data->splittype == TXN_SPLIT_AMOUNT)
	{
		data->remsplit = data->amount - data->sumsplit;
	}
	
	//rules validation	
	sensitive = ((count == nbvalid) && (count > 1)) ? TRUE : FALSE;
	if(data->splittype == TXN_SPLIT_NEW)
		gtk_dialog_set_response_sensitive(GTK_DIALOG(data->dialog), GTK_RESPONSE_SPLIT_SUM, sensitive);
	
	if(data->splittype == TXN_SPLIT_AMOUNT)
	{
		sensitive = arrondi(data->remsplit, 2) != 0.0 ? FALSE : sensitive;
		gtk_dialog_set_response_sensitive(GTK_DIALOG(data->dialog), GTK_RESPONSE_ACCEPT, sensitive);

		if(!data->remsplit)
			g_sprintf(buf, "----");
		else
			g_snprintf(buf, 48, "%.2f", data->remsplit);

		gtk_label_set_label(GTK_LABEL(data->LB_remain), buf);

		g_snprintf(buf, 48, "%.2f", data->amount);
		gtk_label_set_label(GTK_LABEL(data->LB_txnamount), buf);
	}
	
	g_snprintf(buf, 48, "%.2f", data->sumsplit);
	gtk_label_set_text(GTK_LABEL(data->LB_sumsplit), buf);

}


static void ui_txn_split_dialog_inactiveline(GtkWidget *widget, gpointer user_data)
{
struct ui_txn_split_dialog_data *data;
gint line;

	DB( g_print("\n(ui_txn_split_dialog_inactiveline)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(data->nbsplit <= 0)  //1st split always active
		return;

	line = data->nbsplit--;

	DB( g_print("- nbsplit:%d off:%d\n", data->nbsplit, line) );
	
	ui_txn_split_dialog_line_sensitive(line, FALSE, data);
	ui_txn_split_dialog_compute(widget, data);
}


static void ui_txn_split_dialog_activeline(GtkWidget *widget, gpointer user_data)
{
struct ui_txn_split_dialog_data *data;
gint line;

	DB( g_print("\n(ui_txn_split_dialog_activeline)\n") );
	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	line = data->nbsplit;
	if(line >= (TXN_MAX_SPLIT-1))  //bound
		return;

	line = ++data->nbsplit;

	DB( g_print("- nbsplit:%d off:%d\n", data->nbsplit-1, line) );

	
	ui_txn_split_dialog_line_sensitive(line, TRUE, data);

	if(data->splittype == TXN_SPLIT_AMOUNT)
	{
		DB( g_print("- line %d :: affect remain\n", line) );
		g_signal_handler_block(data->ST_amount[line], data->handler_id[line]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount[line]), data->remsplit);
		g_signal_handler_unblock(data->ST_amount[line], data->handler_id[line]);
	}
	
	ui_txn_split_dialog_compute(widget, data);
}


static void ui_txn_split_dialog_get(struct ui_txn_split_dialog_data *data)
{
guint i;
Split *split;
guint32 kcat;
gchar *memo;
gdouble amount;

	DB( g_print("(ui_txn_split_dialog_get)\n") );

	da_transaction_splits_free(data->ope);

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_cat[i]));
		memo = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_memo[i]));
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount[i]));

		if(amount)
		{
			split = da_split_new(kcat, amount, memo);
			
			DB( g_print("- get split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo) );

			da_transaction_splits_append (data->ope, split);
		}
	}
}


static void ui_txn_split_dialog_set(struct ui_txn_split_dialog_data *data)
{
guint count, i;
Split *split;
gchar *txt;

	DB( g_print("(ui_txn_split_dialog_set)\n") );

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		ui_txn_split_dialog_line_sensitive(i, FALSE, data);
		ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat[i]), GLOBALS->h_cat);
		//#1258821		
		//if( data->splittype == TXN_SPLIT_AMOUNT )
		//{
			//if(data->amount > 0.0)
			//	gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->ST_amount[i]), 0.0, G_MAXDOUBLE);
			//else
			//	gtk_spin_button_set_range(GTK_SPIN_BUTTON(data->ST_amount[i]), -G_MAXDOUBLE, 0.0);
		//}
	}

	
	count = da_transaction_splits_count(data->ope);
	data->nbsplit = count > 1 ? count-1 : 0;
	
	DB( g_print("- count = %d\n", count) );
	 
	
	for(i=0;i<count;i++)
	{
		split = data->ope->splits[i];

		DB( g_print("- set split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo) );

		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat[i]), split->kcat);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount[i]), split->amount);
		txt = (split->memo != NULL) ? split->memo : "";
		gtk_entry_set_text(GTK_ENTRY(data->ST_memo[i]), txt);
		ui_txn_split_dialog_line_sensitive(i, TRUE, data);
	}
	
}




static GtkWidget *ui_txn_split_dialog (GtkWidget *parent, Transaction *ope, gdouble amount)
{
struct ui_txn_split_dialog_data data;
GtkWidget *dialog, *content, *mainvbox, *label;
GtkWidget *table, *widget;
gint row, i;


	dialog = gtk_dialog_new_with_buttons (_("Transaction split"),
					    GTK_WINDOW(parent),
					    0,
					    GTK_STOCK_CANCEL,
					    GTK_RESPONSE_CANCEL,
					    NULL);

	data.dialog = dialog;
	data.ope = ope;
	data.amount = amount;
	data.splittype = amount ? TXN_SPLIT_AMOUNT : TXN_SPLIT_NEW;

	//homebank_window_set_icon_from_file(GTK_WINDOW (dialog), "curee.svg");
	//gtk_window_set_icon_name(GTK_WINDOW (dialog), HB_STOCK_CURRENCY);

	gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_REMOVE, GTK_RESPONSE_SPLIT_REM);

	/* sum button must appear only when new split add */
	//#1258821
	//if(data.splittype == TXN_SPLIT_NEW)
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Sum"), GTK_RESPONSE_SPLIT_SUM);


	if(data.splittype == TXN_SPLIT_AMOUNT)
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
	
	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_txn_split_dialog) dialog=%p, inst_data=%p\n", dialog, &data) );

    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);

	//dialog contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
	mainvbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);


	table = gtk_table_new (TXN_MAX_SPLIT, 5, FALSE);
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING/2);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING/2);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	row = 0;
	label = gtk_label_new(_("Category"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_table_attach (GTK_TABLE (table), label, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(_("Memo"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	label = gtk_label_new(_("Amount"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_table_attach (GTK_TABLE (table), label, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		row++;

		data.BT_rem[i] = NULL;
		data.BT_add[i] = NULL;

		if(i > 0)
		{
			widget = gtk_button_new_with_label ("-");
			data.BT_rem[i] = widget;
			gtk_table_attach (GTK_TABLE (table), widget, 0, 1, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
		}

		if( (i < (TXN_MAX_SPLIT-1)) )
		{
			widget = gtk_button_new_with_label ("+");
			data.BT_add[i] = widget;
			gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (0), (GtkAttachOptions) (0), 0, 0);
		}
			
		widget = ui_cat_comboboxentry_new(NULL);
		gtk_widget_set_size_request(widget, 180, -1);
		data.PO_cat[i] = widget;
		gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		widget = make_string(NULL);
		gtk_widget_set_size_request(widget, 180, -1);
		data.ST_memo[i] = widget;
		gtk_table_attach (GTK_TABLE (table), widget, 3, 4, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	
		widget = make_amount(NULL);
		gtk_widget_set_size_request(widget, 100, -1);
		data.ST_amount[i] = widget;
		gtk_table_attach (GTK_TABLE (table), widget, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		
		//connect all our signals
		g_signal_connect (data.PO_cat[i], "changed", G_CALLBACK (ui_txn_split_dialog_compute), &data);
		data.handler_id[i] = g_signal_connect (G_OBJECT (data.ST_amount[i]), "value-changed", G_CALLBACK (ui_txn_split_dialog_compute), &data);
		if(data.BT_rem[i])
			g_signal_connect (data.BT_rem[i], "clicked", G_CALLBACK (ui_txn_split_dialog_inactiveline), GINT_TO_POINTER(i));
		if(data.BT_add[i])
			g_signal_connect (data.BT_add[i], "clicked", G_CALLBACK (ui_txn_split_dialog_activeline), GINT_TO_POINTER(i));
	}
	
	row++;
	label = gtk_label_new(_("Sum of splits:"));
	gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
	gtk_table_attach (GTK_TABLE (table), label, 3, 4, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	label = gtk_label_new(NULL);
	gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
	gtk_misc_set_padding(GTK_MISC(label), 20, 0);
	data.LB_sumsplit = label;
	gtk_table_attach (GTK_TABLE (table), label, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	if( data.splittype == TXN_SPLIT_AMOUNT )
	{
		row++;
		label = gtk_label_new(_("Unassigned:"));
		gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
		gtk_table_attach (GTK_TABLE (table), label, 3, 4, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		label = gtk_label_new(NULL);
		gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
		gtk_misc_set_padding(GTK_MISC(label), 20, 0);
		data.LB_remain = label;
		gtk_table_attach (GTK_TABLE (table), label, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		row++;
		widget = gtk_hseparator_new();
		gtk_table_attach (GTK_TABLE (table), widget, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		row++;
		label = gtk_label_new(_("Transaction amount:"));
		gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
		gtk_table_attach (GTK_TABLE (table), label, 3, 4, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
		label = gtk_label_new(NULL);
		gtk_misc_set_alignment (GTK_MISC(label), 1.0, 0.0);
		gtk_misc_set_padding(GTK_MISC(label), 20, 0);
		data.LB_txnamount = label;
		gtk_table_attach (GTK_TABLE (table), label, 4, 5, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	}


	//setup, init and show dialog
	//ui_cur_manage_dialog_setup(&data);


	ui_txn_split_dialog_set(&data);
	ui_txn_split_dialog_compute(NULL, &data);
	

	//ui_cur_manage_dialog_update(data.LV_cur, NULL);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 480, -1);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (result)
    {
	case GTK_RESPONSE_ACCEPT:
	   //do_application_specific_something ();
	   	ui_txn_split_dialog_get(&data);
	   deftransaction_update(parent, NULL);
	   break;
	case GTK_RESPONSE_SPLIT_REM:
		da_transaction_splits_free(ope);
	  	deftransaction_update(parent, NULL);
		break;
	case GTK_RESPONSE_SPLIT_SUM:	// sum split and alter txn amount   
	   ui_txn_split_dialog_get(&data);
		deftransaction_set_amount_from_split(parent, data.sumsplit);
		deftransaction_update(parent, NULL);
	   break;
	default:
	   //do_nothing_since_dialog_was_cancelled ();
	   break;
    }

	// debug
	#if MYDEBUG == 1
	{
	guint i;

		for(i=0;i<TXN_MAX_SPLIT;i++)
		{
		Split *split = data.ope->splits[i];
			if(data.ope->splits[i] == NULL)
				break;
			g_print(" split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo);
		}
	}
	#endif

	// cleanup and destroy
	//GLOBALS->changes_count += data.change;
	gtk_widget_destroy (dialog);

	return NULL;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void deftransaction_update(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gboolean sensitive, bool;

	DB( g_print("(ui_transaction) update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	//valid & remind are exclusive
	bool  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
	sensitive = bool ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->CM_remind, sensitive);
	if(bool)
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), 0);

	/* disable category if split is set */
	sensitive = (data->ope->flags & (OF_SPLIT)) ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->ST_amount, sensitive);
	gtk_widget_set_sensitive(data->PO_grp, sensitive);

}

void deftransaction_set_amount_from_split(GtkWidget *widget, gdouble amount)
{
struct deftransaction_data *data;

	DB( g_print("(ui_transaction) set_amount_from_split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("- amount=%.2f\n", amount) );

	data->ope->amount = amount;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount);

}


static void deftransaction_set(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *entry;
gchar *tagstr, *txt;

	DB( g_print("(ui_transaction) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_print(" -> ope=%p data=%p tags:%p\n", data->ope, data, entry->tags) );

	//DB( g_print(" set date to %d\n", entry->date) );
	//g_object_set(GTK_DATE_ENTRY(data->PO_date), "date", (guint32)entry->ope_Date);
	gtk_dateentry_set_date(GTK_DATE_ENTRY(data->PO_date), (guint)entry->date);

	txt = (entry->wording != NULL) ? entry->wording : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_word), txt);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), entry->amount);
	//gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_amount), (entry->ope_Flags & OF_INCOME) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_valid), (entry->flags & OF_VALID) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_remind), (entry->flags & OF_REMIND) ? 1 : 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_cheque), (entry->flags & OF_CHEQ2) ? 1 : 0);

	txt = (entry->info != NULL) ? entry->info : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_info), txt);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_grp), entry->kcat);
	ui_pay_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_pay), entry->kpay);

	tagstr = transaction_tags_tostring(entry);

	DB( g_print(" -> tags: '%s'\n", txt) );

	txt = (tagstr != NULL) ? tagstr : "";
	gtk_entry_set_text(GTK_ENTRY(data->ST_tags), txt);
	g_free(tagstr);

	//as we trigger an event on this
	//let's place it at the end to avoid misvalue on the trigger function

	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_acc), entry->kacc);
	ui_acc_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_accto), entry->kxferacc);

	gtk_combo_box_set_active(GTK_COMBO_BOX(data->NU_mode), entry->paymode);

	DB( g_print(" -> acc is: %d\n", gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_acc)) ) );
}


void deftransaction_get(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *entry;
gchar *txt;
gdouble value;
gint active;

	DB( g_print("(ui_transaction) get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	entry = data->ope;

	DB( g_print(" -> ope = %p\n", entry) );

	//DB( g_print(" get date to %d\n", entry->ope_Date) );
	entry->date = gtk_dateentry_get_date(GTK_DATE_ENTRY(data->PO_date));
	//g_object_get(GTK_DATE_ENTRY(data->PO_date), "date", entry->ope_Date);

	//free any previous string
	if(	entry->wording )
	{
		g_free(entry->wording);
		entry->wording = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_word));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->wording = g_strdup(txt);
	}

	entry->paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	if( entry->paymode != PAYMODE_INTXFER )
	{
		//#677351: revert kxferacc to 0
		entry->kxferacc = 0;
	}

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	entry->amount = value;

	/* for internal transfer add, amount must be expense */
	// #617936
	/*
	if( entry->paymode == PAYMODE_INTXFER && data->type == OPERATION_EDIT_ADD )
	{
		if( entry->amount > 0 )
			entry->amount *= -1;
	}
	*/

	//free any previous string
	if(	entry->info )
	{
		g_free(entry->info);
		entry->info = NULL;
	}
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_info));
	// ignore if entry is empty
	if (txt && *txt)
	{
		entry->info = g_strdup(txt);
	}

	entry->kcat     = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_grp));
	entry->kpay     = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_pay));
	entry->kacc     = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
	entry->kxferacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_accto));

	/* tags */
	txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_tags));
	DB( g_print(" -> tags: '%s'\n", txt) );
	transaction_tags_parse(entry, txt);

	/* flags */
	//entry->flags = 0;
	entry->flags &= (OF_SPLIT);	//(split is set in hb_transaction)

	if(	data->type == TRANSACTION_EDIT_ADD || data->type == TRANSACTION_EDIT_INHERIT)
	entry->flags |= OF_ADDED;

	if(	data->type == TRANSACTION_EDIT_MODIFY)
	entry->flags |= OF_CHANGED;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_valid));
	if(active == 1) entry->flags |= OF_VALID;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_remind));
	if(active == 1) entry->flags |= OF_REMIND;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque));
	if(active == 1) entry->flags |= OF_CHEQ2;

	//active = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_amount));
	active = entry->amount > 0 ? TRUE : FALSE;
	if(active == TRUE) entry->flags |= OF_INCOME;

}




static gboolean deftransaction_amount_focusout(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
struct deftransaction_data *data;
gushort paymode;
gdouble amount;

	DB( g_print("(ui_transaction) amount focus-out-event %d\n", gtk_widget_is_focus(widget)) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	paymode    = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));

	// for internal transfer add, amount must be expense by default
	if( paymode == PAYMODE_INTXFER && data->type == TRANSACTION_EDIT_ADD )
	{
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if(amount > 0)
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);
	}

	return FALSE;
}


static void deftransaction_toggleamount(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
guint count, i;
Split *split;
gdouble value;

	DB( g_print("(ui_transaction) toggleamount\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	value *= -1;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), value);
	
	if( data->ope->flags & OF_SPLIT )
	{
		count = da_transaction_splits_count(data->ope);
		DB( g_print("- count = %d\n", count) );
		for(i=0;i<count;i++)
		{
			split = data->ope->splits[i];
			split->amount *= -1;
		}
	}
	
}


static void deftransaction_button_split_cb(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gdouble amount;

	DB( g_print("(ui_transaction) doing split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));

	ui_txn_split_dialog(data->window, data->ope, amount);

}


static void deftransaction_update_transfer(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gboolean sensitive;
guint kacc, kdst;

	DB( g_print("(ui_transaction) update transfer\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	sensitive = TRUE;

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	if(kacc == 0) { sensitive = FALSE; goto end; }

	/* coherent seizure
	 * - target account selected
	 * - source != target
	 * - same currency
	 */
	if( gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode)) == PAYMODE_INTXFER )
	{
		kdst = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_accto));

		if(kdst == 0) { sensitive = FALSE; goto end; }
		if(kdst == kacc) {
			sensitive = FALSE;
			goto end;
		}

		/*
		srcacc = da_acc_get(kacc);
		dstacc = da_acc_get(kdst);
		if(srcacc->kcur != dstacc->kcur) {
			sensitive = FALSE;
		}*/
	}

end:
	DB( g_print(" sensitive %d\n", sensitive) );

	gtk_widget_set_sensitive(gtk_dialog_get_action_area(GTK_DIALOG (data->window)), sensitive);

}


static void deftransaction_update_accto(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
guint kacc;

	DB( g_print("(ui_transaction) update accto\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	kacc = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));

	DB( g_print(" acc is %d\n", kacc) );


	ui_acc_comboboxentry_populate_except(GTK_COMBO_BOX(data->PO_accto), GLOBALS->h_acc, kacc, ACC_LST_INSERT_NORMAL);

	deftransaction_update_transfer(widget, user_data);
}


/*
**
*/
static void deftransaction_paymode(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
gint payment;
gint page;
gboolean sensitive;

	DB( g_print("(ui_transaction) paymode change\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	payment = gtk_combo_box_get_active(GTK_COMBO_BOX(data->NU_mode));
	page = 0;

	/* todo: prefill the cheque number ? */
	if( data->type != TRANSACTION_EDIT_MODIFY )
	{
	gboolean expense = (gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount)) > 0 ? FALSE : TRUE);

		DB( g_print(" -> payment: %d\n", PAYMODE_CHECK) );
		DB( g_print(" -> expense: %d\n", expense) );
		DB( g_print(" -> acc is: %d\n", ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc)) ) );

		if(payment == PAYMODE_CHECK)
		{
			if(expense == TRUE)
			{
			Account *acc;
			gint active = ui_acc_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_acc));
			guint cheque;
			gchar *cheque_str;

				DB( g_print(" -> should fill cheque number for account %d\n", active) );

				if( active != -1 )
				{
					acc = da_acc_get( active );
					cheque = ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_cheque))==TRUE ? acc->cheque2 : acc->cheque1 );
					cheque_str = g_strdup_printf("%d", cheque + 1);
					gtk_entry_set_text(GTK_ENTRY(data->ST_info), cheque_str);
					g_free(cheque_str);
				}
			}

		}
	}


	if(payment == PAYMODE_CHECK)
		page = 1;

	sensitive = (payment == PAYMODE_INTXFER) ? FALSE : TRUE;
	gtk_widget_set_sensitive(data->BT_split, sensitive);

	sensitive = page == 1 ? TRUE : FALSE;
	hb_widget_visible(data->CM_cheque, sensitive);
	
	if(payment == PAYMODE_INTXFER)
	{
		page = 2;
			// for internal transfer add, amount must be expense by default
		if( data->type == TRANSACTION_EDIT_ADD )
		{
			gdouble amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
			if(amount > 0)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), amount *= -1);
		}
	}

	deftransaction_update_accto(widget, user_data);
	DB( g_print(" payment: %d, page: %d\n", payment, page) );

	sensitive = page == 2 ? TRUE : FALSE;
	hb_widget_visible(data->LB_accto, sensitive);
	hb_widget_visible(data->PO_accto, sensitive);

}


static void deftransaction_fillfrom(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;
Transaction *entry;
Archive *arc;
gint n_arc;

	DB( g_print("(ui_transaction) fill from\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	entry = data->ope;

	n_arc = gtk_combo_box_get_active(GTK_COMBO_BOX(data->PO_arc));

	DB( g_print(" fill from %d\n", n_arc) );

	if(n_arc > 0)
	{
		arc = g_list_nth_data(GLOBALS->arc_list, n_arc-1);

		//fill it
		entry->amount	= arc->amount;
		//#1258344 keep the current account if tpl is empty
		if(arc->kacc)
			entry->kacc	= arc->kacc;
		entry->kxferacc	= arc->kxferacc;
		entry->paymode	= arc->paymode;
		entry->flags	= arc->flags;
		entry->kpay	= arc->kpay;
		entry->kcat	= arc->kcat;
		entry->wording =	g_strdup(arc->wording);
		entry->info = NULL;

		DB( g_print(" calls\n") );

		deftransaction_set(widget, NULL);
		deftransaction_paymode(widget, NULL);
		deftransaction_update(widget, NULL);

		gtk_combo_box_set_active(GTK_COMBO_BOX(data->PO_arc), 0);
	}
}


/*
** called from outside
*/
void deftransaction_set_transaction(GtkWidget *widget, Transaction *ope)
{
struct deftransaction_data *data;


	DB( g_print("(ui_transaction) set out transaction\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->ope = ope;

	DB( g_print(" -> ope=%p data=%p\n", data->ope, data) );

	DB( g_print(" -> call init\n") );

	deftransaction_set(widget, NULL);
	deftransaction_paymode(widget, NULL);
	deftransaction_update(widget, NULL);

}


void deftransaction_dispose(GtkWidget *widget, gpointer user_data)
{
struct deftransaction_data *data;

	DB( g_print("(ui_transaction) dispose\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_free(data);
}


static void deftransaction_setup(struct deftransaction_data *data)
{

	DB( g_print("(ui_transaction) setup\n") );

    gtk_window_set_title (GTK_WINDOW (data->window), _(CYA_OPERATION[data->type]));

	ui_pay_comboboxentry_populate(GTK_COMBO_BOX(data->PO_pay), GLOBALS->h_pay);
	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->h_cat);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_acc), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);
	ui_acc_comboboxentry_populate(GTK_COMBO_BOX(data->PO_accto), GLOBALS->h_acc, ACC_LST_INSERT_NORMAL);

	if( (data->type != TRANSACTION_EDIT_MODIFY) && (da_archive_length() > 0))
		make_poparchive_populate(GTK_COMBO_BOX(data->PO_arc), GLOBALS->arc_list);

}


static GtkWidget *deftransaction_make_block1(struct deftransaction_data *data)
{
GtkWidget *table, *hbox, *label, *widget, *image;
gint row;

	table = gtk_table_new (7, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(_("_Date:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = gtk_dateentry_new();
	data->PO_date = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	gtk_widget_set_tooltip_text(widget, _("Date accepted here are:\nday,\nday/month or month/day,\nand complete date into your locale"));

	row++;
	label = make_label(_("_Amount:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_table_attach (GTK_TABLE (table), hbox, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

		widget = gtk_button_new_with_label("+/-");
		data->BT_amount = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		gtk_widget_set_tooltip_text(widget, _("Toggle amount sign"));

		widget = make_amount(label);
		data->ST_amount = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		image = gtk_image_new_from_icon_name ("btn-split", GTK_ICON_SIZE_BUTTON);
		widget = gtk_button_new();
		g_object_set (widget, "image", image, NULL);
		data->BT_split = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
		gtk_widget_set_tooltip_text(widget, _("Category split"));

	row++;
	label = make_label(_("Pa_yment:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_paymode(label);
	data->NU_mode = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic(_("Of notebook _2"));
	data->CM_cheque = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, row, row+1);

	row++;
	label = make_label(_("_Info:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_info = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	label = make_label(_("Acc_ount:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_acc = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("To acc_ount:"), 0.0, 0.5);
	data->LB_accto = label;
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_acc_comboboxentry_new(label);
	data->PO_accto = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);


	return table;
}


static GtkWidget *deftransaction_make_block2(struct deftransaction_data *data)
{
GtkWidget *table, *label, *widget;
gint row;

	table = gtk_table_new (6, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);

	row = 0;
	label = make_label(_("_Payee:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_pay_comboboxentry_new(label);
	data->PO_pay = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);
	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Payee"));

	row++;
	label = make_label(_("_Category:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_cat_comboboxentry_new(label);
	data->PO_grp = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	gtk_widget_set_tooltip_text(widget, _("Autocompletion and direct seizure\nis available for Category"));

	row++;
	label = make_label(_("M_emo:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_memo_entry(label);
	data->ST_word = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Ta_gs:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0,1, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data->ST_tags = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("_Reconciled"));
	data->CM_valid = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Re_mind"));
	data->CM_remind = widget;
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 0, 2, row, row+1);

	return table;
}


GtkWidget *create_deftransaction_window (GtkWindow *parent, gint type)
{
struct deftransaction_data *data;
GtkWidget *window, *content, *hbox, *mainbox, *table;
GtkWidget *alignment;

	DB( g_print("(ui_transaction) new\n") );

	data = g_malloc0(sizeof(struct deftransaction_data));
	if(!data) return NULL;

	
	window = gtk_dialog_new_with_buttons (NULL,
					    GTK_WINDOW (parent),
					    0,
					    NULL,
					    NULL);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)data);
	DB( g_print(" -> window=%p, inst_data=%p\n", window, data) );

	data->window = window;
	data->type = type;

	
	if(type == TRANSACTION_EDIT_MODIFY)
	{
		gtk_dialog_add_buttons (GTK_DIALOG(window),
		    GTK_STOCK_CANCEL,
		    GTK_RESPONSE_REJECT,
			GTK_STOCK_OK,
			GTK_RESPONSE_ACCEPT,
		NULL);
	}
	else
	{
		gtk_dialog_add_buttons (GTK_DIALOG(window),
			GTK_STOCK_CLOSE,
		    GTK_RESPONSE_REJECT,
		    GTK_STOCK_ADD,
		    GTK_RESPONSE_ADD,
		NULL);
	}

	switch(type)
	{
		case TRANSACTION_EDIT_ADD:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_add.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_ADD);
			break;
		case TRANSACTION_EDIT_INHERIT:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_herit.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_HERIT);
			break;
		case TRANSACTION_EDIT_MODIFY:
			//homebank_window_set_icon_from_file(GTK_WINDOW (window), "ope_edit.svg");
			gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_OPE_EDIT);
			break;
	}


	//window contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (content), mainbox, TRUE, TRUE, HB_MAINBOX_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);

	// parameters HB_BOX_SPACING
	hbox = gtk_hbox_new (TRUE, HB_HSPACE_SPACING);
	gtk_box_pack_start (GTK_BOX (mainbox), hbox, FALSE, FALSE, 0);

	// block 1
	table = deftransaction_make_block1(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);


	// block 2
	table = deftransaction_make_block2(data);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0.0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	gtk_box_pack_start (GTK_BOX (hbox), alignment, TRUE, TRUE, 0);

	//fill from
	if( data->type != TRANSACTION_EDIT_MODIFY && da_archive_length() > 0)
	{
	GtkWidget *expander, *widget, *label;

		expander = gtk_expander_new (_("Fill in with a template"));
		gtk_box_pack_start (GTK_BOX (mainbox), expander, FALSE, FALSE, 0);

		hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
		gtk_container_add (GTK_CONTAINER (expander), hbox);

		label = make_label(_("_Template:"), 0, 0.5);
		widget = make_poparchive(label);
		data->PO_arc = widget;
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

		g_signal_connect (data->PO_arc, "changed", G_CALLBACK (deftransaction_fillfrom), NULL);
	}


	//connect all our signals
	g_signal_connect (G_OBJECT (data->ST_amount), "focus-out-event", G_CALLBACK (deftransaction_amount_focusout), data);
	g_signal_connect (G_OBJECT (data->BT_amount), "clicked", G_CALLBACK (deftransaction_toggleamount), NULL);
	g_signal_connect (G_OBJECT (data->BT_split), "clicked", G_CALLBACK (deftransaction_button_split_cb), NULL);

	g_signal_connect (data->NU_mode, "changed", G_CALLBACK (deftransaction_paymode), NULL);
	g_signal_connect (data->CM_cheque, "toggled", G_CALLBACK (deftransaction_paymode), NULL);
	g_signal_connect (data->CM_valid, "toggled", G_CALLBACK (deftransaction_update), NULL);
	g_signal_connect (data->PO_acc, "changed", G_CALLBACK (deftransaction_update_accto), NULL);
	g_signal_connect (data->PO_accto, "changed", G_CALLBACK (deftransaction_update_transfer), NULL);

	//setup, init and show window
	deftransaction_setup(data);

	gtk_window_set_default_size(GTK_WINDOW(window), 640, -1);

	gtk_widget_show_all (window);

	return window;
}
