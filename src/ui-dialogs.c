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

#include "ui-dialogs.h"
#include "list_operation.h"

#include "hb-currency.h"
#include "ui-currency.h"


/* = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

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



/* = = = = = = = = = = = = = = = = = = = = */

/* Confirmation Alert dialog */

gint ui_dialog_msg_confirm_alert(GtkWindow *parent, gchar *title, gchar *secondtext, gchar *actionverb)
{
GtkWidget *dialog;
gint retval;

	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  GTK_MESSAGE_WARNING,
	                                  GTK_BUTTONS_NONE,
	                                  title,
	                                  NULL
	                                  );

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    _("_Cancel"), GTK_RESPONSE_CANCEL,
			actionverb, GTK_RESPONSE_OK,
			NULL);


	if(secondtext)
	{
		g_object_set(GTK_MESSAGE_DIALOG (dialog), "secondary-text", secondtext, NULL);
		//gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), secondtext);
	}
		
	gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
	
	retval = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	return retval;
}




/* Message dialog */




gint ui_dialog_msg_question(GtkWindow *parent, gchar *title, gchar *message_format, ...)
{
GtkWidget *dialog;
gchar* msg = NULL;
va_list args;
gint retval;

	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  GTK_MESSAGE_QUESTION,
	                                  GTK_BUTTONS_YES_NO,
	                                  title,
	                                  NULL
	                                  );

  if (message_format)
    {
      va_start (args, message_format);
      msg = g_strdup_vprintf (message_format, args);
      va_end (args);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);

      g_free (msg);
    }

	gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_NO);
	
	retval = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	return retval;
}

/*
** open a info/error dialog for user information purpose
*/
void ui_dialog_msg_infoerror(GtkWindow *parent, GtkMessageType type, gchar *title, gchar *message_format, ...)
{
GtkWidget *dialog;
gchar* msg = NULL;
va_list args;


	dialog = gtk_message_dialog_new (GTK_WINDOW(parent),
	                                  GTK_DIALOG_DESTROY_WITH_PARENT,
	                                  type,
	                                  GTK_BUTTONS_OK,
	                                  "%s",
	                                  title
	                                  );

  if (message_format)
    {
      va_start (args, message_format);
      msg = g_strdup_vprintf (message_format, args);
      va_end (args);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", msg);

      g_free (msg);
    }

	 gtk_dialog_run (GTK_DIALOG (dialog));
	 gtk_widget_destroy (dialog);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


void ui_dialog_file_statistics(void)
{
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget;
gchar *tmpstr;
gint crow, row, count;

	dialog = gtk_dialog_new_with_buttons (_("File statistics"),
		GTK_WINDOW (GLOBALS->mainwindow),
		0,
		_("_Close"),
		GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_window_set_default_size (GTK_WINDOW(dialog), HB_MINWIDTH_LIST, -1);
	
	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: file title
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(NULL);
	tmpstr = g_path_get_basename(GLOBALS->xhb_filepath);
	gtk_label_set_text(GTK_LABEL(label), tmpstr);
	g_free(tmpstr);
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row = 1;
	label = make_label_widget(_("Account"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	count = da_acc_length ();
	ui_label_set_integer(GTK_LABEL(widget), count);
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Transaction"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	count = da_transaction_length();
	ui_label_set_integer(GTK_LABEL(widget), count);
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Payee"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	count = da_pay_length ();
	ui_label_set_integer(GTK_LABEL(widget), count);
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Category"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	count = da_cat_length ();
	ui_label_set_integer(GTK_LABEL(widget), count);
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("Assignment"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_label(NULL, 1.0, 0.5);
	count = da_asg_length ();
	ui_label_set_integer(GTK_LABEL(widget), count);
	gtk_widget_set_hexpand(widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	gtk_widget_show_all(content_grid);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	if(result == GTK_RESPONSE_ACCEPT)
	{

	}

	// cleanup and destroy
	gtk_widget_destroy (dialog);

}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

struct dialog_currency_data
{
	GtkWidget   *window;
	GtkWidget   *LB_currency;
	GtkWidget   *BT_change;
	Currency4217 *curfmt;
};

static void ui_dialog_upgrade_choose_currency_change_action(GtkWidget *widget, gpointer user_data)
{
struct dialog_currency_data *data = user_data;
Currency4217 *curfmt;

	data->curfmt = NULL;

	curfmt = ui_cur_select_dialog_new(GTK_WINDOW(data->window), CUR_SELECT_MODE_BASE);
	if( curfmt != NULL )
	{
	gchar label[128];
	gchar *name;
		
		DB( g_printf("- user selected: '%s' '%s'\n", curfmt->curr_iso_code, curfmt->name) );

		data->curfmt = curfmt;

		name = curfmt->name;

		g_snprintf(label, 127, "%s - %s", curfmt->curr_iso_code, name);
		gtk_label_set_text (GTK_LABEL(data->LB_currency), label);
	}
}


static void ui_dialog_upgrade_choose_currency_fill(struct dialog_currency_data *data)
{
Currency *cur;
gchar label[128];

	data->curfmt = NULL;

	cur = da_cur_get (GLOBALS->kcur);

	g_snprintf(label, 127, "%s - %s", cur->iso_code, cur->name);
	gtk_label_set_text (GTK_LABEL(data->LB_currency), label);
}



void ui_dialog_upgrade_choose_currency(void)
{
struct dialog_currency_data data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget;
gint crow, row;

	dialog = gtk_dialog_new_with_buttons (_("Upgrade"),
		GTK_WINDOW (GLOBALS->mainwindow),
		0,
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_OK"), GTK_RESPONSE_ACCEPT,
		NULL);

	data.window = dialog;

	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: file title
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	row = 0;
	label = make_label(_("Select a base currency"), 0, 0);
	gimp_label_set_attributes(GTK_LABEL(label), 
		PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, 
		PANGO_ATTR_SCALE,  PANGO_SCALE_LARGE, 
		-1);
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 3, 1);

	row++;
	label = make_label(
		_("Starting v5.1, HomeBank can manage several currencies\n" \
		  "if the currency below is not correct, please change it:"), 0, 0);
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 3, 1);

	row++;
	label = make_label_widget(_("Currency:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
	widget = make_label (NULL, 0, 0.5);
	data.LB_currency = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);
	widget = gtk_button_new_with_mnemonic (_("_Change"));
	data.BT_change = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	g_signal_connect (G_OBJECT (data.BT_change), "clicked", G_CALLBACK (ui_dialog_upgrade_choose_currency_change_action), &data);


	ui_dialog_upgrade_choose_currency_fill(&data);

	gtk_widget_show_all(content_grid);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	if(result == GTK_RESPONSE_ACCEPT)
	{

		if( data.curfmt != NULL )
		{
			hbfile_replace_basecurrency(data.curfmt);
		}
	}

	// in any case set every accounts to base currency
	GList *list;
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *acc = list->data;

		account_set_currency(acc, GLOBALS->kcur);
		list = g_list_next(list);
	}
	g_list_free(list);
	
	// cleanup and destroy
	gtk_widget_destroy (dialog);

}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */




static void ui_file_chooser_add_filter(GtkFileChooser *chooser, gchar *name, gchar *pattern)
{
	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, name);
	gtk_file_filter_add_pattern (filter, pattern);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER(chooser), filter);
}


gboolean ui_file_chooser_qif(GtkWindow *parent, gchar **storage_ptr)
{
GtkWidget *chooser;
gboolean retval;

	DB( g_print("(homebank) chooser save qif\n") );

	chooser = gtk_file_chooser_dialog_new (
					_("Export as QIF"),
					GTK_WINDOW(parent),
					GTK_FILE_CHOOSER_ACTION_SAVE,
					_("_Cancel"), GTK_RESPONSE_CANCEL,
					_("_Save"), GTK_RESPONSE_ACCEPT,
					NULL);

	//todo: change this ?
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_export);
	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("QIF files"), "*.[Qq][Ii][Ff]");
	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("All files"), "*");

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
	gchar *tmpfilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	
		*storage_ptr = hb_util_filename_new_with_extension(tmpfilename, "qif");
		g_free(tmpfilename);
		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}



/*
** open a file chooser dialog and store filename to GLOBALS if OK
*/
gboolean ui_file_chooser_csv(GtkWindow *parent, GtkFileChooserAction action, gchar **storage_ptr, gchar *name)
{
GtkWidget *chooser;
gchar *title;
gchar *button;
gboolean retval;
gchar *path;

	DB( g_print("(hombank) csvfile chooser csv %d\n", action) );

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
		title = _("Import from CSV");
		button = _("_Open");
		path = PREFS->path_import;
	}
	else
	{
		title = _("Export as CSV");
		button = _("_Save");
		path = PREFS->path_export;
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(parent),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					_("_Cancel"), GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), path);

	if(name != NULL)
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), name);

	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("CSV files"), "*.[Cc][Ss][Vv]");
	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("All files"), "*");

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
	gchar *tmpfilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
	
		if( action == GTK_FILE_CHOOSER_ACTION_SAVE )
		{
			*storage_ptr = hb_util_filename_new_with_extension(tmpfilename, "csv");
			g_free(tmpfilename);
		}
		else
		{
			*storage_ptr = tmpfilename;
		}
		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}

/*
** open a file chooser dialog and store filename to GLOBALS if OK
*/
gboolean ui_file_chooser_xhb(GtkFileChooserAction action, gchar **storage_ptr)
{
GtkWidget *chooser;
gchar *title;
gchar *button;
gboolean retval;

	DB( g_print("(ui-dialog) file chooser xhb %d\n", action) );

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
		title = _("Open HomeBank file");
		button = _("_Open");
	}
	else
	{
		title = _("Save HomeBank file as");
		button = _("_Save");
	}

	chooser = gtk_file_chooser_dialog_new (title,
					GTK_WINDOW(GLOBALS->mainwindow),
					action,	//GTK_FILE_CHOOSER_ACTION_OPEN,
					_("_Cancel"), GTK_RESPONSE_CANCEL,
					button, GTK_RESPONSE_ACCEPT,
					NULL);

	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("HomeBank files"), "*.[Xx][Hh][Bb]");
	ui_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), _("All files"), "*");

	if( action == GTK_FILE_CHOOSER_ACTION_OPEN )
	{
	    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), PREFS->path_hbfile);
	}
	else /* save */
	{
	gchar *basename, *dirname;
		
		basename = g_path_get_basename(GLOBALS->xhb_filepath);
		dirname  = g_path_get_dirname (GLOBALS->xhb_filepath);
		//gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), GLOBALS->xhb_filepath);

		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser), dirname);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(chooser), basename);
		
		g_free(dirname);
		g_free(basename);	
	}

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
		*storage_ptr = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}

/*
**
*/
gboolean ui_file_chooser_folder(GtkWindow *parent, gchar *title, gchar **storage_ptr)
{
GtkWidget *chooser;
gboolean retval;

	DB( g_print("(ui-dialog) folder chooser\n") );

	chooser = gtk_file_chooser_dialog_new (title,
					parent,
					GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					_("_Cancel"), GTK_RESPONSE_CANCEL,
					_("_Open"), GTK_RESPONSE_ACCEPT,
					NULL);

	DB( g_print(" - set folder %s\n", *storage_ptr) );

	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(chooser), *storage_ptr);

	retval = FALSE;
	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
    gchar *filename;

		//nb: filename must be freed with g_free
	    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));

		DB( g_print("- folder %s\n", filename) );

		//todo: dangerous to do this here, review please !
		g_free(*storage_ptr);
		*storage_ptr = filename;

		DB( g_print("- folder stored: %s\n", *storage_ptr) );


		retval = TRUE;
	}

	gtk_widget_destroy (chooser);

	return retval;
}



/*
** request the user to save last change
*/
gboolean ui_dialog_msg_savechanges(GtkWidget *widget, gpointer user_data)
{
gboolean retval = TRUE;
GtkWidget *dialog = NULL;


  	if(GLOBALS->changes_count)
	{
	gint result;

		dialog = gtk_message_dialog_new
		(
			GTK_WINDOW(GLOBALS->mainwindow),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_WARNING,
			//GTK_MESSAGE_INFO,
			GTK_BUTTONS_NONE,
			_("Save changes to the file before closing?")
		);

		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
			_("If you don't save, changes will be permanently lost.\nNumber of changes: %d."),
			GLOBALS->changes_count
			);

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
		    _("Close _without saving"), 0,
		    _("_Cancel"), 1,
			_("_Save"), 2,
			NULL);

		gtk_dialog_set_default_response(GTK_DIALOG( dialog ), 2);

	  result = gtk_dialog_run( GTK_DIALOG( dialog ) );
	  gtk_widget_destroy( dialog );

	  if(result == 1 || result == GTK_RESPONSE_DELETE_EVENT)
		{
	  	retval = FALSE;
		}
		else
		{
	  if(result == 2)
	  {
		DB( g_print(" + should quick save %s\n", GLOBALS->xhb_filepath) );
		homebank_save_xml(GLOBALS->xhb_filepath);
	  }
		}



	}
	return retval;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

struct xfer_data
{
	GtkWidget   *window;
	GtkWidget	*radio[2];
	GtkWidget	*srctreeview;
	GtkWidget	*treeview;
};


static void ui_dialog_transaction_xfer_select_child_cb(GtkWidget *radiobutton, gpointer user_data)
{
struct xfer_data *data;
GtkTreeSelection *selection;
gboolean btnew, sensitive;
gint count;
	

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(radiobutton), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(import) account type toggle\n") );

	btnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->radio[0]));
	gtk_widget_set_sensitive(data->treeview, btnew^1);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->treeview));
	count = gtk_tree_selection_count_selected_rows(selection);

	
	sensitive = (btnew || count > 0) ? TRUE : FALSE;

	DB( g_print("test count %d btnew %d sensitive %d\n", count, btnew, sensitive) );

	
	gtk_dialog_set_response_sensitive(GTK_DIALOG(data->window), GTK_RESPONSE_ACCEPT, sensitive);

}

static void ui_dialog_transaction_xfer_select_child_selection_cb(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_dialog_transaction_xfer_select_child_cb(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}


Transaction *ui_dialog_transaction_xfer_select_child(Transaction *stxn, GList *matchlist)
{
struct xfer_data data;
GtkWidget *window, *content, *mainvbox, *vbox, *sw, *label, *LB_several;
GtkTreeModel		 *newmodel;
GtkTreeIter			 newiter;
Transaction *retval = NULL;

	window = gtk_dialog_new_with_buttons (
    			_("Select among possible transactions..."),
    			GTK_WINDOW (GLOBALS->mainwindow),
			    0,
			    _("_Cancel"),
			    GTK_RESPONSE_REJECT,
			    _("_OK"),
			    GTK_RESPONSE_ACCEPT,
			    NULL);

	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	data.window = window;

	//gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 494);

	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
		mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), SPACING_SMALL);

	// source listview
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	//gtk_widget_set_size_request(sw, -1, HB_MINWIDTH_LIST/2);

	data.srctreeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(data.srctreeview)), GTK_SELECTION_NONE);
	gtk_container_add (GTK_CONTAINER (sw), data.srctreeview);
	gtk_box_pack_start (GTK_BOX (mainvbox), sw, TRUE, TRUE, 0);


	// actions 
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (mainvbox), vbox, FALSE, TRUE, SPACING_SMALL);

	label = make_label(_("Select an action:"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	data.radio[0] = gtk_radio_button_new_with_label (NULL, _("create a new transaction"));
	gtk_box_pack_start (GTK_BOX (vbox), data.radio[0], FALSE, FALSE, 0);

	data.radio[1] = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (data.radio[0]), _("select an existing transaction"));
	gtk_box_pack_start (GTK_BOX (vbox), data.radio[1], FALSE, FALSE, 0);


	label = make_label(_(
		"HomeBank has found some transaction that may be " \
		"the associated transaction for the internal transfer."), 0.0, 0.5
		);
	LB_several = label;
	gimp_label_set_attributes (GTK_LABEL (label),
                             PANGO_ATTR_SCALE,  PANGO_SCALE_SMALL,
                             -1);
	gtk_box_pack_start (GTK_BOX (mainvbox), label, FALSE, FALSE, SPACING_SMALL);

	// target listview
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	data.treeview = create_list_transaction(LIST_TXN_TYPE_DETAIL, PREFS->lst_ope_columns);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(data.treeview)), GTK_SELECTION_SINGLE);
	gtk_container_add (GTK_CONTAINER (sw), data.treeview);
	gtk_box_pack_start (GTK_BOX (mainvbox), sw, TRUE, TRUE, 0);

	/* populate source */
	if( stxn != NULL )
	{
		newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data.srctreeview));
		gtk_list_store_clear (GTK_LIST_STORE(newmodel));
	
		gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

		gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
		LST_DSPOPE_DATAS, stxn,
		-1);
	}

	/* populate target */
	newmodel = gtk_tree_view_get_model(GTK_TREE_VIEW(data.treeview));
	gtk_list_store_clear (GTK_LIST_STORE(newmodel));

	GList *tmplist = g_list_first(matchlist);
	while (tmplist != NULL)
	{
	Transaction *tmp = tmplist->data;

		/* append to our treeview */
			gtk_list_store_append (GTK_LIST_STORE(newmodel), &newiter);

			gtk_list_store_set (GTK_LIST_STORE(newmodel), &newiter,
			LST_DSPOPE_DATAS, tmp,
			-1);

		//DB( g_print(" - fill: %s %.2f %x\n", item->wording, item->amount, (unsigned int)item->same) );

		tmplist = g_list_next(tmplist);
	}


	g_signal_connect (data.radio[0], "toggled", G_CALLBACK (ui_dialog_transaction_xfer_select_child_cb), NULL);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.treeview)), "changed", G_CALLBACK (ui_dialog_transaction_xfer_select_child_selection_cb), NULL);

	gtk_widget_show_all(mainvbox);

	//initialize
	gtk_widget_set_sensitive (data.radio[1], TRUE);
	if( g_list_length (matchlist) <= 0 )
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data.radio[0]), TRUE);
		gtk_widget_set_sensitive (data.radio[1], FALSE);
		gtk_widget_set_visible (LB_several, FALSE);
	}
	else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data.radio[1]), TRUE);
		gtk_widget_set_visible (LB_several, TRUE);
	}
	
	ui_dialog_transaction_xfer_select_child_cb(data.radio[0], NULL);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	if(result == GTK_RESPONSE_ACCEPT)
	{
	gboolean bnew;

		bnew = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data.radio[0]));
		if( bnew == FALSE )
		{
		GtkTreeSelection *selection;
		GtkTreeModel		 *model;
		GtkTreeIter			 iter;

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data.treeview));
			if (gtk_tree_selection_get_selected(selection, &model, &iter))
			{
				gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &retval, -1);
			}
		}

	}

	// cleanup and destroy
	gtk_widget_destroy (window);

	return retval;
}



