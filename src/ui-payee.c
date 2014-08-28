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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-payee.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ui_pay_comboboxentry_get_name:
 *
 * get the name of the active payee or -1
 *
 * Return value: a new allocated name tobe freed with g_free
 *
 */
gchar *
ui_pay_comboboxentry_get_name(GtkComboBox *entry_box)
{
gchar *cbname;
gchar *name = NULL;

	cbname = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));

	if( cbname != NULL)
	{
		name = g_strdup(cbname);
		g_strstrip(name);
	}

	return name;
}


/**
 * ui_pay_comboboxentry_get_key_add_new:
 *
 * get the key of the active payee
 * and create the payee if it do not exists
 *
 * Return value: the key or 0
 *
 */
guint32
ui_pay_comboboxentry_get_key_add_new(GtkComboBox *entry_box)
{
gchar *name;
Payee *item;

	name = ui_pay_comboboxentry_get_name(entry_box);

	item = da_pay_get_by_name(name);
	if( item == NULL )
	{
		/* automatic add */
		//todo: check prefs + ask the user here 1st time
		item = da_pay_malloc();
		item->name = g_strdup(name);
		da_pay_append(item);
		ui_pay_comboboxentry_add(entry_box, item);
	}

	g_free(name);

	return item->key;
}

/**
 * ui_pay_comboboxentry_get_key:
 *
 * get the key of the active payee
 *
 * Return value: the key or 0
 *
 */
guint32
ui_pay_comboboxentry_get_key(GtkComboBox *entry_box)
{
gchar *name;
Payee *item;

	name = ui_pay_comboboxentry_get_name(entry_box);
	item = da_pay_get_by_name(name);
	g_free(name);

	if( item != NULL )
		return item->key;

	return 0;
}

gboolean
ui_pay_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key)
{
Payee *item;

	if( key > 0 )
	{
		item = da_pay_get(key);
		if( item != NULL)
		{
			gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), item->name);
			return TRUE;
		}
	}
	gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), "");
	return FALSE;
}




/**
 * ui_pay_comboboxentry_add:
 *
 * Add a single element (useful for dynamics add)
 *
 * Return value: --
 *
 */
void
ui_pay_comboboxentry_add(GtkComboBox *entry_box, Payee *pay)
{
	if( pay->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter  iter;

		model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, pay->name, -1);
	}
}

static void
ui_pay_comboboxentry_populate_ghfunc(gpointer key, gpointer value, struct payPopContext *ctx)
{
GtkTreeIter  iter;
Payee *pay = value;

	if( ( pay->key != ctx->except_key ) )
	{
		gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter, 0, pay->name, -1);
	}
}

/**
 * ui_pay_comboboxentry_populate:
 *
 * Populate the list and completion
 *
 * Return value: --
 *
 */
void
ui_pay_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash)
{
	ui_pay_comboboxentry_populate_except(entry_box, hash, -1);
}

void
ui_pay_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key)
{
GtkTreeModel *model;
GtkEntryCompletion *completion;
struct payPopContext ctx;

    DB( g_print ("ui_pay_comboboxentry_populate\n") );

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));
	completion = gtk_entry_get_completion(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));

	/* keep our model alive and detach from comboboxentry and completion */
	g_object_ref(model);
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), NULL);
	gtk_entry_completion_set_model (completion, NULL);

	/* clear and populate */
	ctx.model = model;
	ctx.except_key = except_key;
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_hash_table_foreach(hash, (GHFunc)ui_pay_comboboxentry_populate_ghfunc, &ctx);

	/* reatach our model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	gtk_entry_completion_set_model (completion, model);
	g_object_unref(model);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

}



static gint
ui_pay_comboboxentry_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint retval = 0;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, 0, &name1, -1);
    gtk_tree_model_get(model, b, 0, &name2, -1);

	retval = hb_string_utf8_compare(name1, name2);

    g_free(name2);
    g_free(name1);

  	return retval;
  }


static void
ui_pay_comboboxentry_test (GtkCellLayout   *cell_layout,
		   GtkCellRenderer *cell,
		   GtkTreeModel    *tree_model,
		   GtkTreeIter     *iter,
		   gpointer         data)
{
gchar *name;

	gtk_tree_model_get(tree_model, iter,
		0, &name,
		-1);

	if( !name )
		g_object_set(cell, "text", _("(no payee)"), NULL);
	else
		g_object_set(cell, "text", name, NULL);

}

/**
 * ui_pay_comboboxentry_new:
 *
 * Create a new payee comboboxentry
 *
 * Return value: the new widget
 *
 */
GtkWidget *
ui_pay_comboboxentry_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *comboboxentry;
GtkEntryCompletion *completion;
GtkCellRenderer    *renderer;

	store = gtk_list_store_new (1,
		G_TYPE_STRING
		);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_pay_comboboxentry_compare_func, NULL, NULL);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
	g_object_set(completion, "text-column", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (completion), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (completion), renderer, "text", 0, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (completion),
					    renderer,
					    ui_pay_comboboxentry_test,
					    NULL, NULL);

	// dothe same for combobox

	comboboxentry = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(comboboxentry), 0);

	gtk_cell_layout_clear(GTK_CELL_LAYOUT (comboboxentry));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (comboboxentry), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (comboboxentry), renderer, "text", 0, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (comboboxentry),
					    renderer,
					    ui_pay_comboboxentry_test,
					    NULL, NULL);



	gtk_entry_set_completion (GTK_ENTRY (gtk_bin_get_child(GTK_BIN (comboboxentry))), completion);

	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), comboboxentry);

	gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_COMBO, -1);

	return comboboxentry;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
ui_pay_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFPAY_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFPAY_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_pay_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
Payee *entry1, *entry2;

    gtk_tree_model_get(model, a, LST_DEFPAY_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFPAY_DATAS, &entry2, -1);

    return hb_string_utf8_compare(entry1->name, entry2->name);
}

static void
ui_pay_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Payee *entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &entry, -1);

	if(entry->key == 0)
		name = _("(no payee)");
	else
		name = entry->name;

	#if MYDEBUG
		string = g_strdup_printf ("%d > %s [ft=%d im=%d]", entry->key, name, entry->filter, entry->imported);
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}



/* = = = = = = = = = = = = = = = = */


void
ui_pay_listview_add(GtkTreeView *treeview, Payee *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFPAY_TOGGLE, FALSE,
			LST_DEFPAY_DATAS, item,
			-1);
	}
}

guint32
ui_pay_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Payee *item;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &item, -1);

		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}

void
ui_pay_listview_remove_selected(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
}


static void ui_pay_listview_populate_ghfunc(gpointer key, gpointer value, GtkTreeModel *model)
{
GtkTreeIter	iter;
Payee *item = value;

	DB( g_print(" populate: %p\n", key) );

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		LST_DEFPAY_TOGGLE	, FALSE,
		LST_DEFPAY_DATAS, item,
		-1);
}

void ui_pay_listview_populate(GtkWidget *view)
{
GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	g_hash_table_foreach(GLOBALS->h_pay, (GHFunc)ui_pay_listview_populate_ghfunc, model);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}


GtkWidget *
ui_pay_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(
		NUM_LST_DEFPAY,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFPAY_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_pay_listview_toggled_cb), store);

	}

	// column 2: name
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_pay_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFPAY_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// treeview attribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, ui_pay_listview_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFPAY_DATAS, GTK_SORT_ASCENDING);

	return treeview;
}




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ui_pay_manage_dialog_load_csv:
 *
 */
static void
ui_pay_manage_dialog_load_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
gchar *filename = NULL;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_pay_manage_dialog) load csv - data %p\n", data) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		payee_load_csv(filename);
		//todo: add error message

		g_free( filename );
		ui_pay_listview_populate(data->LV_pay);
	}
}

/**
 * ui_pay_manage_dialog_save_csv:
 *
 */
static void
ui_pay_manage_dialog_save_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
gchar *filename = NULL;

	DB( g_print("(ui_pay_manage_dialog) save csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		payee_save_csv(filename);
		g_free( filename );
	}
}


/**
 * ui_pay_manage_dialog_add:
 *
 */
static void
ui_pay_manage_dialog_add(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
Payee *item;
gchar *name;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) add (data=%p)\n", data) );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));

	item = payee_append_if_new(name);
	if( item )
	{
		ui_pay_listview_add(GTK_TREE_VIEW(data->LV_pay), item);
		data->change++;
	}

	gtk_entry_set_text(GTK_ENTRY(data->ST_name), "");
}


static void ui_pay_manage_dialog_modify_entry_cb(GtkEditable *editable, gpointer user_data)
{
GtkDialog *window = user_data;
const gchar *buffer;

	buffer = gtk_entry_get_text(GTK_ENTRY(editable));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


/*
** modify
*/
static void ui_pay_manage_dialog_modify(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
GtkWidget *window, *content, *mainvbox, *getwidget;
guint32 key;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) modify %p\n", data) );

	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));
	if( key > 0 )
	{
	Payee *item;

		item = da_pay_get( key );

		window = gtk_dialog_new_with_buttons (_("Modify..."),
						    GTK_WINDOW (data->window),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		content = gtk_dialog_get_content_area(GTK_DIALOG (window));
		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		getwidget = gtk_entry_new();
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);
		gtk_widget_show_all(mainvbox);

		g_signal_connect (G_OBJECT (getwidget), "changed", G_CALLBACK (ui_pay_manage_dialog_modify_entry_cb), window);

		gtk_entry_set_text(GTK_ENTRY(getwidget), item->name);
		gtk_widget_grab_focus (getwidget);

		gtk_entry_set_activates_default (GTK_ENTRY(getwidget), TRUE);

		gtk_dialog_set_default_response(GTK_DIALOG( window ), GTK_RESPONSE_ACCEPT);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			name = gtk_entry_get_text(GTK_ENTRY(getwidget));

			/* ignore if entry is empty */
			if (name && *name)
			{
				if( payee_rename(item, name) )
				{
					//to redraw the active entry
					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_pay));
					data->change++;
				}
				else
				{
					ui_dialog_msg_infoerror(GTK_WINDOW(window), GTK_MESSAGE_ERROR,
						_("Error"),
						_("Cannot rename this Payee,\n"
						"from '%s' to '%s',\n"
						"this name already exists."),
						item->name,
						name
						);

				}
			}
	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}


static void ui_pay_manage_dialog_move_entry_cb(GtkComboBox *widget, gpointer user_data)
{
GtkDialog *window = user_data;
gchar *buffer;

	buffer = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (widget))));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


/*
** move
*/
static void ui_pay_manage_dialog_move(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
GtkWidget *window, *content, *mainvbox;
GtkWidget *getwidget;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) move %p\n", data) );

	// get selection ...
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Payee *entry;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &entry, -1);

		window = gtk_dialog_new_with_buttons (_("Move to..."),
						    GTK_WINDOW (data->window),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		content = gtk_dialog_get_content_area(GTK_DIALOG (window));
		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		getwidget = ui_pay_comboboxentry_new(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);

		//gtk_combo_box_set_active(GTK_COMBO_BOX(getwidget), oldpos);
		gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, FALSE);

		gtk_widget_show_all(mainvbox);

		g_signal_connect (G_OBJECT (getwidget), "changed", G_CALLBACK (ui_pay_manage_dialog_move_entry_cb), window);

		//data->tmp_list = g_list_sort(data->tmp_list, (GCompareFunc)ui_pay_manage_dialog_list_sort);
		ui_pay_comboboxentry_populate_except(GTK_COMBO_BOX(getwidget), GLOBALS->h_pay, entry->key);
		gtk_widget_grab_focus (getwidget);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gint result;
		gchar *npn;

			npn = ui_pay_comboboxentry_get_name(GTK_COMBO_BOX(getwidget)),

			result = ui_dialog_msg_question(
				GTK_WINDOW(window),
				_("Move this payee to another one ?"),
				_("This will replace '%s' by '%s',\n"
				  "and then remove '%s'"),
				entry->name,
				npn,
				entry->name,
				NULL
				);

			if( result == GTK_RESPONSE_YES )
			{
			Payee *payee;
			guint newpayee;

				newpayee = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(getwidget));

				gtk_combo_box_get_active(GTK_COMBO_BOX(getwidget));

				DB( g_print(" -> should move %d - %s to %d - %s\n", entry->key, entry->name, newpayee, npn ) );

				payee_move(entry->key, newpayee);

				// remove the old payee
				da_pay_remove(entry->key);
				ui_pay_listview_remove_selected(GTK_TREE_VIEW(data->LV_pay));

				// add the new payee to listview
				payee = da_pay_get(newpayee);
				if(payee)
					ui_pay_listview_add(GTK_TREE_VIEW(data->LV_pay), payee);
				data->change++;

			}

		}

		// cleanup and destroy
		gtk_widget_destroy (window);

	}

}


/*
** remove the selected payee to our treeview and temp GList
*/
static void ui_pay_manage_dialog_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
guint32 key;
Payee *item;
gint result;
gboolean do_remove;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(ui_pay_manage_dialog) remove (data=%p)\n", data) );

	do_remove = TRUE;
	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));
	if( key > 0 )
	{
		if( payee_is_used(key) == TRUE )
		{
			item = da_pay_get(key);

			result = ui_dialog_msg_question(
				GTK_WINDOW(data->window),
				_("Remove a payee ?"),
				_("If you remove '%s', archive and transaction referencing this payee\n"
				"will set place to 'no payee'"),
				item->name,
				NULL
				);

			if( result == GTK_RESPONSE_YES )
			{
				payee_move(key, 0);
			}
			else if( result == GTK_RESPONSE_NO )
			{
				do_remove = FALSE;
			}
		}

		if( do_remove )
		{
			da_pay_remove(key);
			ui_pay_listview_remove_selected(GTK_TREE_VIEW(data->LV_pay));
			data->change++;
		}

	}
}


/*
**
*/
static void ui_pay_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
gboolean sensitive;
guint32 key;

	DB( g_print("\n(ui_pay_manage_dialog) cursor changed\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));

	sensitive = (key > 0) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->BT_mov, sensitive);
	gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

}


/*
**
*/
static void ui_pay_manage_dialog_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_pay_manage_dialog_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

static void ui_pay_manage_dialog_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	{
		ui_pay_manage_dialog_modify(GTK_WIDGET(treeview), NULL);
	}
}

/*
**
*/
GtkWidget *ui_pay_manage_dialog (void)
{
struct ui_pay_manage_dialog_data data;
GtkWidget *window, *content, *mainvbox, *treeview, *scrollwin, *vbox, *table;
GtkWidget *separator;
gint row;

	window = gtk_dialog_new_with_buttons (_("Manage Payees"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;
	data.change = 0;


	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "payee.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_PAYEE);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(ui_pay_manage_dialog) window=%p, inst_data=%p\n", window, &data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	//window contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainvbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

    //our table
	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	row = 0;
	data.ST_name = gtk_entry_new ();
	gtk_table_attach (GTK_TABLE (table), data.ST_name, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.BT_add = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//list
	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_table_attach (GTK_TABLE (table), scrollwin, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 	//treeview = (GtkWidget *)ui_pay_manage_dialog_list_new(FALSE);
	treeview = ui_pay_listview_new(FALSE);
	gtk_widget_set_size_request(treeview, HB_MINWIDTH_LIST, -1);
 	data.LV_pay = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), vbox, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	data.BT_rem = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_rem, FALSE, FALSE, 0);

	data.BT_mod = gtk_button_new_from_stock(GTK_STOCK_EDIT);
	//data.BT_mod = gtk_button_new_with_mnemonic(_("_Modify"));
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_mod, FALSE, FALSE, 0);

	data.BT_mov = gtk_button_new_with_mnemonic(_("_Move"));
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_mov, FALSE, FALSE, 0);

	separator = gtk_hseparator_new();
	gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, HB_BOX_SPACING);


	data.BT_import = gtk_button_new_with_mnemonic(_("_Import"));
	//data.BT_import = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_import, FALSE, FALSE, 0);

	data.BT_export = gtk_button_new_with_mnemonic(_("E_xport"));
	//data.BT_export = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	gtk_box_pack_start (GTK_BOX (vbox), data.BT_export, FALSE, FALSE, 0);



	//connect all our signals
	g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (ui_pay_manage_dialog_add), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_pay)), "changed", G_CALLBACK (ui_pay_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_pay), "row-activated", G_CALLBACK (ui_pay_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_add), "clicked", G_CALLBACK (ui_pay_manage_dialog_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_mod), "clicked", G_CALLBACK (ui_pay_manage_dialog_modify), NULL);
	g_signal_connect (G_OBJECT (data.BT_mov), "clicked", G_CALLBACK (ui_pay_manage_dialog_move), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_pay_manage_dialog_remove), NULL);

	g_signal_connect (G_OBJECT (data.BT_import), "clicked", G_CALLBACK (ui_pay_manage_dialog_load_csv), NULL);
	g_signal_connect (G_OBJECT (data.BT_export), "clicked", G_CALLBACK (ui_pay_manage_dialog_save_csv), NULL);

	//setup, init and show window
	ui_pay_listview_populate(data.LV_pay);
	ui_pay_manage_dialog_update(data.LV_pay, NULL);

	//gtk_window_resize(GTK_WINDOW(window), 200, 320);

	gtk_widget_show_all (window);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (window));

	switch (result)
    {
	case GTK_RESPONSE_ACCEPT:
	   //do_application_specific_something ();
	   break;
	default:
	   //do_nothing_since_dialog_was_cancelled ();
	   break;
    }


	// cleanup and destroy
	GLOBALS->changes_count += data.change;
	gtk_widget_destroy (window);

	return NULL;
}



