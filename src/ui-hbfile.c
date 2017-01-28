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

#include "ui-hbfile.h"
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





/*
** get widgets contents from the selected account
*/
static void defhbfile_get(GtkWidget *widget, gpointer user_data)
{
struct defhbfile_data *data;
gchar	*owner;
guint32	vehicle;
gint	smode, weekday, nbdays;

	DB( g_print("(ui-hbfile) get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	// get values
	owner = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_owner));
	vehicle   = ui_cat_comboboxentry_get_key(GTK_COMBO_BOX(data->PO_grp));
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0])) )
		smode = 0;
	else
		smode = 1;
	weekday = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NU_weekday));
	nbdays  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NU_nbdays));

	// check for changes
	if(strcasecmp(owner, GLOBALS->owner))
		data->change++;
	if(vehicle != GLOBALS->vehicle_category)
		data->change++;
	if(smode != GLOBALS->auto_smode)
		data->change++;
	if(weekday != GLOBALS->auto_weekday)
		data->change++;
	if(nbdays != GLOBALS->auto_nbdays)
		data->change++;

	// update
	if (owner && *owner)
		hbfile_change_owner(g_strdup(owner));

	GLOBALS->vehicle_category = vehicle;
	GLOBALS->auto_smode   = smode;
	GLOBALS->auto_weekday = weekday;
	GLOBALS->auto_nbdays  = nbdays;

	DB( g_print(" -> owner %s\n", GLOBALS->owner) );
	DB( g_print(" -> ccgrp %d\n", GLOBALS->vehicle_category) );
	DB( g_print(" -> smode %d\n", GLOBALS->auto_smode) );
	DB( g_print(" -> weekday %d\n", GLOBALS->auto_weekday) );
	DB( g_print(" -> nbdays %d\n", GLOBALS->auto_nbdays) );

}


/*
** set widgets contents from the selected account
*/
static void defhbfile_set(GtkWidget *widget, gpointer user_data)
{
struct defhbfile_data *data;

	DB( g_print("(ui-hbfile) set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print(" -> ccgrp %d\n", GLOBALS->vehicle_category) );
	DB( g_print(" -> autoinsert %d\n", GLOBALS->auto_nbdays) );

	if(GLOBALS->owner) gtk_entry_set_text(GTK_ENTRY(data->ST_owner), GLOBALS->owner);
	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_grp), GLOBALS->vehicle_category);
	
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->radio[GLOBALS->auto_smode]), TRUE);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NU_nbdays), GLOBALS->auto_nbdays);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NU_weekday), GLOBALS->auto_weekday);


}

static void defhbfile_toggle(GtkRadioButton *radiobutton, gpointer user_data)
{
struct defhbfile_data *data;
gboolean sensitive;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("\n(defhbfile_data) toggle\n") );

	sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0])); 

	DB( g_print(" -> radio[0]=%d %s\n", sensitive, sensitive ? "add until" : "add every x") );
	
	gtk_widget_set_sensitive (data->LB_nbdays, !sensitive);
	gtk_widget_set_sensitive (data->LB_weekday, sensitive);

	gtk_widget_set_sensitive (data->NU_nbdays, !sensitive);
	gtk_widget_set_sensitive (data->NU_weekday, sensitive);

}


/*
**
*/
static gboolean defhbfile_cleanup(struct defhbfile_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(ui-hbfile) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{
		defhbfile_get(data->ST_owner, NULL);


		DB( g_print(" -> GLOBAL change = %d\n", GLOBALS->changes_count) );

		DB( g_print(" -> we update, change = %d\n", data->change) );


		GLOBALS->changes_count += data->change;
	}
	return doupdate;
}

/*
**
*/
static void defhbfile_setup(struct defhbfile_data *data)
{
	DB( g_print("(ui-hbfile) setup\n") );

	data->change = 0;

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_grp), GLOBALS->h_cat);

	defhbfile_set(data->ST_owner, NULL);

}


GtkWidget *create_defhbfile_dialog (void)
{
struct defhbfile_data data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget;
gint crow, row;

	dialog = gtk_dialog_new_with_buttons (_("File properties"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
				_("_Cancel"),
				GTK_RESPONSE_REJECT,
				_("_OK"),
				GTK_RESPONSE_ACCEPT,
				NULL);

	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(defaccount) dialog=%p, inst_data=%p\n", dialog, &data) );

	gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_PROPERTIES);
	gtk_window_set_resizable(GTK_WINDOW (dialog), FALSE);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));	// return a vbox

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: General
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("General"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("_Owner:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string(label);
	data.ST_owner = widget;
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	// group :: Scheduled transaction
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Scheduled transaction"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 4, 1);

	row = 1;
	widget = gtk_radio_button_new_with_label (NULL, _("add until"));
	data.radio[0] = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	widget = make_numeric(NULL, 1, 28);
	data.NU_weekday = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	label = make_label(_("of each month (excluded)"), 0.0, 0.5);
	data.LB_weekday = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 3, row, 1, 1);
	
	row++;
	widget = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (data.radio[0]), _("add"));
	data.radio[1] = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

	widget = make_numeric(NULL, 0, 366);
	data.NU_nbdays = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	//TRANSLATORS: there is a spinner on the left of this label, and so you have 0....x days in advance the current date
	label = make_label(_("days in advance the current date"), 0.0, 0.5);
	data.LB_nbdays = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 3, row, 1, 1);

	// group :: Scheduled transaction
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Vehicle cost"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("_Category:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = ui_cat_comboboxentry_new(label);
	data.PO_grp = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);


	//connect all our signals
	g_signal_connect (dialog, "destroy", G_CALLBACK (gtk_widget_destroyed), &dialog);

	g_signal_connect (data.radio[0], "toggled", G_CALLBACK (defhbfile_toggle), NULL);

	
	//setup, init and show window
	defhbfile_setup(&data);
	//defhbfile_update(data.LV_arc, NULL);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	// cleanup and destroy
	defhbfile_cleanup(&data, result);
	gtk_widget_destroy (dialog);

	return dialog;
}
