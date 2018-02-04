/*	HomeBank -- Free, easy, personal accounting for everyone.
 *	Copyright (C) 1995-2018 Maxime DOYEN
 *
 *	This file is part of HomeBank.
 *
 *	HomeBank is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	HomeBank is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.	If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-assist-start.h"
#include "dsp_mainwindow.h"
#include "ui-currency.h"


#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;

extern gchar *CYA_ACC_TYPE[];

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
on_assistant_apply (GtkWidget *widget, gpointer user_data)
{
struct assist_start_data *data = user_data;
Account *item;
gdouble value;

	DB( g_print("\n[ui-start] apply\n") );


	/* set owner */
	gchar *owner = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_owner));
	if (owner && *owner)
	{
		hbfile_change_owner(g_strdup(owner));
		GLOBALS->changes_count++;
	}

	if( data->curfmt != NULL )
	{
		hbfile_replace_basecurrency(data->curfmt);
	}

	/* load preset categories */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_load)))
	{
		if(data->pathfilename != NULL)
		{
		gchar *error;
			category_load_csv(data->pathfilename, &error);
			//DB( g_print(" -> loaded=%d\n", ok) );
		}
	}

	/* initialise an account */
	item = da_acc_malloc();

	gchar *txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
	if (txt && *txt)
	{
		item->name = g_strdup(txt);
	}

	item->kcur = GLOBALS->kcur;
	
	item->type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));

	item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));

	gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_initial));
	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
	item->initial = value;

	gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_minimum));
	value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
	item->minimum = value;

	da_acc_append(item);
	GLOBALS->changes_count++;

	//our global list has changed, so update the treeview
	//todo: #1693998 crappy to do this here
	account_compute_balances ();
	ui_mainwindow_update(GLOBALS->mainwindow, GINT_TO_POINTER(UF_TITLE+UF_SENSITIVE+UF_BALANCE+UF_REFRESHALL));

}

static void
on_assistant_close_cancel (GtkWidget *widget, gpointer user_data)
{
struct assist_start_data *data = user_data;

	DB( g_print("\n[ui-start] close/cancel\n") );


	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	gtk_widget_destroy (data->window);

	g_free(data->pathfilename);

	g_free(data);

}

static void
on_assistant_prepare (GtkWidget *widget, GtkWidget *page, gpointer user_data)
{
struct assist_start_data *data = user_data;
	gint current_page, n_pages;
  gchar *title;

	DB( g_print("\n[ui-start] prepare\n") );


  current_page = gtk_assistant_get_current_page (GTK_ASSISTANT (widget));
  n_pages = gtk_assistant_get_n_pages (GTK_ASSISTANT (widget));

  title = g_strdup_printf (_("New HomeBank file (%d of %d)"), current_page + 1, n_pages);
  gtk_window_set_title (GTK_WINDOW (widget), title);
  g_free (title);


	switch( current_page  )
	{
		case 1:
		{
			gchar **langs = (gchar **)g_get_language_names ();
			gchar *txt = g_strjoinv(", ", langs);

			DB( g_print("%s\n", txt) );;

			gtk_label_set_label(GTK_LABEL(data->TX_lang), txt);
			g_free(txt);


			gchar *lang;
			data->pathfilename = category_find_preset(&lang);
			if(data->pathfilename != NULL)
			{
				gtk_label_set_label(GTK_LABEL(data->TX_file), lang);
				gtk_widget_show(data->CM_load);
				gtk_widget_show(data->ok_image);
				gtk_widget_hide(data->ko_image);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_load), TRUE);
			}
			else
			{
				gtk_widget_hide(data->CM_load);
				gtk_label_set_label(GTK_LABEL(data->TX_file), _("Not found"));
				gtk_widget_show(data->ko_image);
				gtk_widget_hide(data->ok_image);
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_load), FALSE);
			}


		}
			break;


	}

}

static void
on_entry_changed (GtkWidget *widget, gpointer data)
{
  GtkAssistant *assistant = GTK_ASSISTANT (data);
  GtkWidget *current_page;
  gint page_number;
  const gchar *text;

  page_number = gtk_assistant_get_current_page (assistant);
  current_page = gtk_assistant_get_nth_page (assistant, page_number);
  text = gtk_entry_get_text (GTK_ENTRY (widget));

  if (text && *text)
    gtk_assistant_set_page_complete (assistant, current_page, TRUE);
  else
    gtk_assistant_set_page_complete (assistant, current_page, FALSE);
}


static void ui_start_assistant_property_change_action(GtkWidget *widget, gpointer user_data)
{
struct assist_start_data *data;
struct curSelectContext selectCtx;
	
	DB( g_print("\n[ui-start] property_change_action\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	data->curfmt = NULL;
	ui_cur_select_dialog_new(GTK_WINDOW(data->window), CUR_SELECT_MODE_BASE, &selectCtx);
	if( selectCtx.cur_4217 != NULL )
	{
	Currency4217 *curfmt;
	gchar label[128];
	gchar *name;
		
		curfmt = selectCtx.cur_4217;
		
		DB( g_printf("- user selected: '%s' '%s'\n", curfmt->curr_iso_code, curfmt->name) );

		data->curfmt = curfmt;

		name = curfmt->name;

		g_snprintf(label, 127, "%s - %s", curfmt->curr_iso_code, name);
		gtk_label_set_text (GTK_LABEL(data->LB_currency), label);
	}
}


static void
ui_start_assistant_property_fill (GtkWidget *assistant, struct assist_start_data *data)
{
Currency *cur;
gchar label[128];

	DB( g_print("\n[ui-start] property_fill\n") );


	gtk_entry_set_text(GTK_ENTRY(data->ST_owner), g_get_real_name ());

	cur = da_cur_get (GLOBALS->kcur);

	g_snprintf(label, 127, "%s - %s", cur->iso_code, cur->name);
	gtk_label_set_text (GTK_LABEL(data->LB_currency), label);
	
}


static void
ui_start_assistant_property_create (GtkWidget *assistant, struct assist_start_data *data)
{
GtkWidget *table, *label, *widget;
gint row;
	
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	//gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_MEDIUM);
	gtk_widget_set_valign (table, GTK_ALIGN_CENTER);

	row = 0;
	label = make_label_widget(_("_Owner:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_string(label);
	data->ST_owner = widget;
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 2, 1);

	row++;
	label = make_label_widget(_("Currency:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_label (NULL, 0, 0.5);
	data->LB_currency = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);
	widget = gtk_button_new_with_mnemonic (_("_Change"));
	data->BT_change = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 3, row, 1, 1);

	g_signal_connect (G_OBJECT (data->ST_owner), "changed", G_CALLBACK (on_entry_changed), assistant);
	g_signal_connect (G_OBJECT (data->BT_change), "clicked", G_CALLBACK (ui_start_assistant_property_change_action), data);

	
	gtk_widget_show_all (table);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), table);

	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), table, _("File properties"));
	gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), table, GTK_ASSISTANT_PAGE_INTRO);

}

static void
ui_start_assistant_create_page2 (GtkWidget *assistant, struct assist_start_data *data)
{
GtkWidget *box, *hbox, *label, *table, *widget;
gint row;

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (box), SPACING_MEDIUM);

	table = gtk_grid_new ();
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL*2);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM*2);

	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);

	row = 0;
	label = make_label_group(_("System detection"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("Languages:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_label(NULL, 0.0, 0.5);
	data->TX_lang = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Preset file:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (table), hbox, 2, row, 1, 1);

	widget = gtk_image_new_from_icon_name(ICONNAME_HB_FILE_VALID, GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->ok_image = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	widget = gtk_image_new_from_icon_name(ICONNAME_HB_FILE_INVALID, GTK_ICON_SIZE_LARGE_TOOLBAR);
	data->ko_image = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	widget = make_label(NULL, 0.0, 0.5);
	data->TX_file = widget;
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Initialize my categories with this file"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
	data->CM_load = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 3, 1);

	gtk_widget_show_all (box);

	gtk_widget_hide(data->ok_image);
	gtk_widget_hide(data->ko_image);

	gtk_assistant_append_page (GTK_ASSISTANT (assistant), box);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), box, TRUE);
	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), box, _("Preset categories"));

}

static void
ui_start_assistant_create_page3 (GtkWidget *assistant, struct assist_start_data *data)
{
GtkWidget *box, *label, *widget, *table;
gint row;

  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
  gtk_container_set_border_width (GTK_CONTAINER (box), SPACING_MEDIUM);


	table = gtk_grid_new ();
	//gtk_container_set_border_width (GTK_CONTAINER (table), SP_BORDER);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);

	row = 0;
	label = make_label_group(_("Informations"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("_Name:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_string(label);
	data->ST_name = widget;
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	g_signal_connect (G_OBJECT (widget), "changed",
		    G_CALLBACK (on_entry_changed), assistant);


	row++;
	label = make_label_widget(_("_Type:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_cycle(label, CYA_ACC_TYPE);
	data->CY_type = widget;
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("N_umber:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_string(label);
	data->ST_number = widget;
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

//other

	//row = 0;
	row++;
	label = make_label_group(_("Balances"));
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("_Initial:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_amount(label);
	data->ST_initial = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Overdrawn at:"));
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = make_amount(label);
	data->ST_minimum = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	gtk_widget_show_all (box);
	gtk_assistant_append_page (GTK_ASSISTANT (assistant), box);

	gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), box, _("Create an account"));
}


static void
ui_start_assistant_create_page4 (GtkWidget *assistant, struct assist_start_data *data)
{
  GtkWidget *label;


  label = gtk_label_new (_("This is a confirmation page, press 'Apply' to apply changes"));

  gtk_widget_show (label);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), label);
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), label, GTK_ASSISTANT_PAGE_CONFIRM);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), label, _("Confirmation"));


}





GtkWidget*
ui_start_assistant (void)
{
struct assist_start_data *data;
GtkWidget *assistant, *page;

	DB( g_print("\n[ui-start] new\n") );


	data = g_malloc0(sizeof(struct assist_start_data));
	if(!data) return NULL;

	assistant = gtk_assistant_new ();
	data->window = assistant;

	//store our window private data
	g_object_set_data(G_OBJECT(assistant), "inst_data", (gpointer)data);
	//DB( g_print("** (import) window=%x, inst_data=%x\n", assistant, data) );

	gtk_window_set_default_size (GTK_WINDOW (assistant), 400, -1);

	gtk_window_set_modal(GTK_WINDOW (assistant), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(assistant), GTK_WINDOW(GLOBALS->mainwindow));

	ui_start_assistant_property_create (assistant, data);
	ui_start_assistant_create_page2 (assistant, data);
	ui_start_assistant_create_page3 (assistant, data);
	ui_start_assistant_create_page4 (assistant, data);

	ui_start_assistant_property_fill(assistant, data);
	
	page = gtk_assistant_get_nth_page(GTK_ASSISTANT (assistant), 0);
	gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), page, TRUE);


	g_signal_connect (G_OBJECT (assistant), "cancel",
			G_CALLBACK (on_assistant_close_cancel), data);
	g_signal_connect (G_OBJECT (assistant), "close",
			G_CALLBACK (on_assistant_close_cancel), data);
	g_signal_connect (G_OBJECT (assistant), "apply",
			G_CALLBACK (on_assistant_apply), data);
	g_signal_connect (G_OBJECT (assistant), "prepare",
			G_CALLBACK (on_assistant_prepare), data);

	gtk_widget_show (assistant);

	return assistant;
}
