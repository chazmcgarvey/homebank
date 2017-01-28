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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty ofdeftransaction_amountchanged
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-split.h"
#include "ui-transaction.h"
#include "ui-archive.h"
#include "gtk-dateentry.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-account.h"
#include "hb-split.h"




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


#define GTK_RESPONSE_SPLIT_SUM 10880
#define GTK_RESPONSE_SPLIT_REM 10888


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void ui_split_dialog_filter_text_handler (GtkEntry    *entry,
                          const gchar *text,
                          gint         length,
                          gint        *position,
                          gpointer     data)
{
GtkEditable *editable = GTK_EDITABLE(entry);
gint i, count=0;
gchar *result = g_new0 (gchar, length+1);

  for (i=0; i < length; i++)
  {
    if (text[i]=='|')
      continue;
    result[count++] = text[i];
  }


  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (ui_split_dialog_filter_text_handler),
                                     data);
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (ui_split_dialog_filter_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert-text");

  g_free (result);
}


void ui_split_dialog_line_sensitive(guint line, gboolean sensitive, gpointer user_data)
{
struct ui_split_dialog_data *data = user_data;

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


void ui_split_dialog_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data = user_data;
gint i, count, nbvalid;
//gint j;
gchar buf[48];
gboolean sensitive, active;
//guint32 cat[TXN_MAX_SPLIT];
gdouble amt[TXN_MAX_SPLIT];
gboolean valid[TXN_MAX_SPLIT];

	DB( g_print("\n(ui_split_dialog_compute)\n") );

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
		sensitive = hb_amount_round(data->remsplit, 2) != 0.0 ? FALSE : sensitive;
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


void ui_split_dialog_inactiveline(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
gint line;

	DB( g_print("\n(ui_split_dialog_inactiveline)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if(data->nbsplit <= 0)  //1st split always active
		return;

	line = data->nbsplit--;

	DB( g_print("- nbsplit:%d off:%d\n", data->nbsplit, line) );
	
	ui_split_dialog_line_sensitive(line, FALSE, data);
	ui_split_dialog_compute(widget, data);
}


void ui_split_dialog_activeline(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
gint line;

	DB( g_print("\n(ui_split_dialog_activeline)\n") );
	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	line = data->nbsplit;
	if(line >= (TXN_MAX_SPLIT-1))  //bound
		return;

	line = ++data->nbsplit;

	DB( g_print("- nbsplit:%d off:%d\n", data->nbsplit-1, line) );

	
	ui_split_dialog_line_sensitive(line, TRUE, data);

	if(data->splittype == TXN_SPLIT_AMOUNT)
	{
		DB( g_print("- line %d :: affect remain\n", line) );
		g_signal_handler_block(data->ST_amount[line], data->handler_id[line]);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount[line]), data->remsplit);
		g_signal_handler_unblock(data->ST_amount[line], data->handler_id[line]);
	}
	
	ui_split_dialog_compute(widget, data);
}


void ui_split_dialog_get(struct ui_split_dialog_data *data)
{
guint i;
Split *split;
guint32 kcat;
gchar *memo;
gdouble amount;

	DB( g_print("(ui_split_dialog_get)\n") );

	da_splits_free(data->splits);

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_cat[i]));
		memo = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_memo[i]));
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount[i]));

		if(amount)
		{
			split = da_split_new(kcat, amount, memo);
			
			DB( g_print("- get split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo) );

			da_splits_append (data->splits, split);
		}
	}
}


void ui_split_dialog_set(struct ui_split_dialog_data *data)
{
guint count, i;
Split *split;
gchar *txt;

	DB( g_print("(ui_split_dialog_set)\n") );

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		ui_split_dialog_line_sensitive(i, FALSE, data);
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

	
	count = da_splits_count(data->splits);
	data->nbsplit = count > 1 ? count-1 : 0;
	
	DB( g_print("- count = %d\n", count) );
	 
	
	for(i=0;i<count;i++)
	{
		split = data->splits[i];

		DB( g_print("- set split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo) );

		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat[i]), split->kcat);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount[i]), split->amount);
		txt = (split->memo != NULL) ? split->memo : "";
		gtk_entry_set_text(GTK_ENTRY(data->ST_memo[i]), txt);
		ui_split_dialog_line_sensitive(i, TRUE, data);
	}
	
}




GtkWidget *ui_split_dialog (GtkWidget *parent, Split *ope_splits[], gdouble amount, void (update_callbackFunction(GtkWidget*, gdouble)))
{
struct ui_split_dialog_data data;
GtkWidget *dialog, *content, *mainvbox, *label;
GtkWidget *table, *widget;
gint row, i;


	dialog = gtk_dialog_new_with_buttons (_("Transaction splits"),
					    GTK_WINDOW(parent),
					    0,
					    _("_Cancel"),
					    GTK_RESPONSE_CANCEL,
					    NULL);

	data.dialog = dialog;
	data.splits = ope_splits;
	data.amount = amount;
	data.splittype = amount ? TXN_SPLIT_AMOUNT : TXN_SPLIT_NEW;

	gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Remove"), GTK_RESPONSE_SPLIT_REM);

	/* sum button must appear only when new split add */
	//#1258821
	//if(data.splittype == TXN_SPLIT_NEW)
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Sum"), GTK_RESPONSE_SPLIT_SUM);


	if(data.splittype == TXN_SPLIT_AMOUNT)
		gtk_dialog_add_button(GTK_DIALOG(dialog), _("_OK"), GTK_RESPONSE_ACCEPT);
	
	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_split_dialog) dialog=%p, inst_data=%p\n", dialog, &data) );

    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);

	//dialog contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), SPACING_MEDIUM);


	table = gtk_grid_new ();
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL/2);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM/2);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	row = 0;
	label = gtk_label_new(_("Category"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);

	label = gtk_label_new(_("Memo"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 3, row, 1, 1);

	label = gtk_label_new(_("Amount"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 4, row, 1, 1);

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		row++;

		data.BT_rem[i] = NULL;
		data.BT_add[i] = NULL;

		if(i > 0)
		{
			widget = gtk_button_new_with_label ("-");
			data.BT_rem[i] = widget;
			gtk_grid_attach (GTK_GRID (table), widget, 0, row, 1, 1);
		}

		if( (i < (TXN_MAX_SPLIT-1)) )
		{
			widget = gtk_button_new_with_label ("+");
			data.BT_add[i] = widget;
			gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);
		}
			
		widget = ui_cat_comboboxentry_new(NULL);
		data.PO_cat[i] = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

		widget = make_string(NULL);
		gtk_widget_set_hexpand (widget, TRUE);
		data.ST_memo[i] = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 3, row, 1, 1);
	
		widget = make_amount(NULL);
		data.ST_amount[i] = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 4, row, 1, 1);
		
		//connect all our signals
		g_signal_connect (data.PO_cat[i], "changed", G_CALLBACK (ui_split_dialog_compute), &data);
		g_signal_connect (data.ST_memo[i], "insert-text", G_CALLBACK(ui_split_dialog_filter_text_handler), NULL);
		data.handler_id[i] = g_signal_connect (G_OBJECT (data.ST_amount[i]), "value-changed", G_CALLBACK (ui_split_dialog_compute), &data);
		if(data.BT_rem[i])
			g_signal_connect (data.BT_rem[i], "clicked", G_CALLBACK (ui_split_dialog_inactiveline), GINT_TO_POINTER(i));
		if(data.BT_add[i])
			g_signal_connect (data.BT_add[i], "clicked", G_CALLBACK (ui_split_dialog_activeline), GINT_TO_POINTER(i));
	}
	
	row++;
	label = gtk_label_new(_("Sum of splits:"));
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (table), label, 3, row, 1, 1);
	widget = gtk_label_new(NULL);
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	data.LB_sumsplit = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 4, row, 1, 1);

	if( data.splittype == TXN_SPLIT_AMOUNT )
	{
		row++;
		label = gtk_label_new(_("Unassigned:"));
		gtk_widget_set_halign (label, GTK_ALIGN_END);
		gtk_grid_attach (GTK_GRID (table), label, 3, row, 1, 1);
		widget = gtk_label_new(NULL);
		gtk_widget_set_halign (widget, GTK_ALIGN_END);
		gtk_widget_set_margin_left(widget, 20);
		gtk_widget_set_margin_right(widget, 20);
		data.LB_remain = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 4, row, 1, 1);

		row++;
		widget = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_grid_attach (GTK_GRID (table), widget, 4, row, 1, 1);

		row++;
		label = gtk_label_new(_("Transaction amount:"));
		gtk_widget_set_halign (label, GTK_ALIGN_END);
		gtk_grid_attach (GTK_GRID (table), label, 3, row, 1, 1);
		widget = gtk_label_new(NULL);
		gtk_widget_set_halign (widget, GTK_ALIGN_END);
		gtk_widget_set_margin_left(widget, 20);
		gtk_widget_set_margin_right(widget, 20);
		data.LB_txnamount = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 4, row, 1, 1);
	}


	//setup, init and show dialog
	//ui_cur_manage_dialog_setup(&data);


	ui_split_dialog_set(&data);
	ui_split_dialog_compute(NULL, &data);
	

	//ui_cur_manage_dialog_update(data.LV_cur, NULL);

	gtk_window_set_default_size(GTK_WINDOW(dialog), 480, -1);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (result)
    {
	case GTK_RESPONSE_ACCEPT:
	   //do_application_specific_something ();
	   	ui_split_dialog_get(&data);
		update_callbackFunction(parent,data.sumsplit);
	   break;
	case GTK_RESPONSE_SPLIT_REM:
		da_splits_free(ope_splits);
		update_callbackFunction(parent,data.sumsplit);
		break;
	case GTK_RESPONSE_SPLIT_SUM:	// sum split and alter txn amount   
		ui_split_dialog_get(&data);
		update_callbackFunction(parent,data.sumsplit);
	   break;
	default:
	   //do_nothing_since_dialog_was_cancelled ();
	   break;
    }

	// debug
	/*#if MYDEBUG == 1
	{
	guint i;

		for(i=0;i<TXN_MAX_SPLIT;i++)
		{
		Split *split = data.ope_splits[i];
			if(data.ope_splits[i] == NULL)
				break;
			g_print(" split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo);
		}
	}
	#endif*/

	// cleanup and destroy
	//GLOBALS->changes_count += data.change;
	gtk_widget_destroy (dialog);

	return NULL;
}

