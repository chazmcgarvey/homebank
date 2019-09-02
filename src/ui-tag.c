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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty ofdeftransaction_amountchanged
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "homebank.h"

#include "ui-tag.h"


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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

//TODO: still used in rep_time
void
ui_tag_combobox_populate(GtkComboBoxText *combobox)
{
GList *ltag, *list;
	
	//populate template
	hbtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), 0, "----");
	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);

	ltag = list = tag_glist_sorted(1);
	while (list != NULL)
	{
	Tag *item = list->data;
	
		DB( g_print(" populate: %d\n", item->key) );

		hbtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combobox), item->key, item->name);
		list = g_list_next(list);
	}

	g_list_free(ltag);
	
}


GtkWidget *
ui_tag_combobox_new(GtkWidget *label)
{
GtkWidget *combobox;

	combobox = hbtk_combo_box_new(label);
	return combobox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void ui_tag_popover_cb_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
GtkTreeSelection *treeselection;
GtkTreeModel *model;
GtkTreeIter iter;
GtkEntry *entry = user_data;

	if( GTK_IS_ENTRY(entry) )
	{
		treeselection = gtk_tree_view_get_selection(tree_view);
		if( gtk_tree_selection_get_selected(treeselection, &model, &iter) )
		{
		Tag *item;

			gtk_tree_model_get(model, &iter, LST_DEFTAG_DATAS, &item, -1);

			hbtk_entry_tag_name_append(GTK_ENTRY(user_data), item->name);
		}
	}
}


GtkWidget *
ui_tag_popover_list(GtkWidget *entry)
{
GtkWidget *box, *menubutton, *image, *scrollwin, *treeview;

	menubutton = gtk_menu_button_new ();
	image = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(menubutton), image);

	//gtk_menu_button_set_direction (GTK_MENU_BUTTON(menubutton), GTK_ARROW_DOWN );
	//gtk_widget_set_halign (menubutton, GTK_ALIGN_END);
	gtk_widget_show_all(menubutton);

	//GtkWidget *template = ui_popover_tpl_create(data);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start(GTK_BOX(box), scrollwin, TRUE, TRUE, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	treeview = ui_tag_listview_new(FALSE);
	//data.LV_tag = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_widget_show_all(box);

	gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(treeview), TRUE);

	
	GtkWidget *popover = create_popover (menubutton, box, GTK_POS_BOTTOM);
	gtk_widget_set_size_request (popover, HB_MINWIDTH_LIST, HB_MINHEIGHT_LIST);

	gtk_menu_button_set_popover(GTK_MENU_BUTTON(menubutton), popover);

	ui_tag_listview_populate(treeview, 0);

	g_signal_connect (treeview, "row-activated", G_CALLBACK (ui_tag_popover_cb_row_activated), entry);
	g_signal_connect_swapped(treeview, "row-activated", G_CALLBACK(gtk_popover_popdown), popover);
	
	return menubutton;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
ui_tag_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFTAG_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFTAG_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_tag_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint retval = 0;
Tag *entry1, *entry2;
//gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFTAG_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFTAG_DATAS, &entry2, -1);

	retval = hb_string_utf8_compare(entry1->name, entry2->name);

    return retval;
}


static void
ui_tag_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Tag *entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, LST_DEFTAG_DATAS, &entry, -1);
	if(entry->name == NULL)
		name = _("(none)");		// can never occurs !
	else
		name = entry->name;

	#if MYDEBUG
		string = g_strdup_printf ("[%d] %s", entry->key, name );
		g_object_set(renderer, "text", string, NULL);
		g_free(string);
	#else
		g_object_set(renderer, "text", name, NULL);
	#endif

}



/* = = = = = = = = = = = = = = = = */

/**
 * tag_list_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_tag_listview_add(GtkTreeView *treeview, Tag *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFTAG_TOGGLE, FALSE,
			LST_DEFTAG_DATAS, item,
			-1);

		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}

guint32
ui_tag_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Tag *item;

		gtk_tree_model_get(model, &iter, LST_DEFTAG_DATAS, &item, -1);
		
		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}

void
ui_tag_listview_remove_selected(GtkTreeView *treeview)
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


void ui_tag_listview_populate(GtkWidget *view, gint insert_type)
{
GtkTreeModel *model;
GtkTreeIter	iter;
GList *ltag, *list;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	//g_hash_table_foreach(GLOBALS->h_tag, (GHFunc)ui_tag_listview_populate_ghfunc, model);
	ltag = list = g_hash_table_get_values(GLOBALS->h_tag);
	while (list != NULL)
	{
	Tag *item = list->data;
	
		DB( g_print(" populate: %d\n", item->key) );

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFTAG_TOGGLE	, FALSE,
			LST_DEFTAG_DATAS, item,
			-1);

		list = g_list_next(list);
	}
	g_list_free(ltag);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}


GtkWidget *
ui_tag_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(NUM_LST_DEFTAG,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);

	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFTAG_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_tag_listview_toggled_cb), store);

	}

	// column 2: name
	column = gtk_tree_view_column_new();

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);

	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_tag_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFTAG_DATAS), NULL);

	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);
	
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_tag_listview_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void ui_tag_manage_filter_text_handler (GtkEntry    *entry,
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
    if (text[i]==' ')
      continue;
    result[count++] = text[i];
  }


  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (ui_tag_manage_filter_text_handler),
                                     data);
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (ui_tag_manage_filter_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

  g_free (result);
}



/**
 * ui_tag_manage_dialog_add:
 *
 */
static void
ui_tag_manage_dialog_add(GtkWidget *widget, gpointer user_data)
{
struct ui_tag_manage_dialog_data *data;
Tag *item;
gchar *name;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) add (data=%p)\n", data) );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));

	item = da_tag_malloc ();
	item->name = g_strdup(name);

	g_strstrip(item->name);
	
	if( strlen(item->name) > 0 )
	{
		if( da_tag_append(item) )
		{
			ui_tag_listview_add(GTK_TREE_VIEW(data->LV_tag), item);
			data->change++;
		}
	}
	else
		da_tag_free (item);
		
	gtk_entry_set_text(GTK_ENTRY(data->ST_name), "");
}


static void ui_tag_manage_dialog_edit_entry_cb(GtkEditable *editable, gpointer user_data)
{
GtkDialog *window = user_data;
const gchar *buffer;

	buffer = gtk_entry_get_text(GTK_ENTRY(editable));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


static void ui_tag_manage_dialog_edit(GtkWidget *dowidget, gpointer user_data)
{
struct ui_tag_manage_dialog_data *data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget;
GtkWidget *ST_name;
gint crow, row;
guint32 key;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(dowidget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) modify %p\n", data) );

	key = ui_tag_listview_get_selected_key(GTK_TREE_VIEW(data->LV_tag));
	if( key > 0 )
	{
	Tag *item;

		item = da_tag_get( key );

		dialog = gtk_dialog_new_with_buttons (_("Edit..."),
						    GTK_WINDOW (data->window),
						    0,
						    _("_Cancel"),
						    GTK_RESPONSE_REJECT,
						    _("_OK"),
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

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
	
		//label = make_label_group(_("General"));
		//gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

		row = 1;
		label = make_label_widget(_("_Name:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
		widget = gtk_entry_new();
		ST_name = widget;
		gtk_widget_set_hexpand(widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		g_signal_connect (G_OBJECT (ST_name), "changed", G_CALLBACK (ui_tag_manage_dialog_edit_entry_cb), dialog);

		gtk_widget_show_all(content_grid);


		gtk_dialog_set_default_response(GTK_DIALOG( dialog ), GTK_RESPONSE_ACCEPT);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			// 1: manage renaming
			name = gtk_entry_get_text(GTK_ENTRY(ST_name));
			// ignore if item is empty
			if (name && *name)
			{
				if( tag_rename(item, name) )
				{
					//to redraw the active entry
					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_tag));
					data->change++;
				}
				else
				{
					ui_dialog_msg_infoerror(GTK_WINDOW(dialog), GTK_MESSAGE_ERROR,
						_("Error"),
						_("Cannot rename this Tag,\n"
						"from '%s' to '%s',\n"
						"this name already exists."),
						item->name,
						name
						);

				}
			}


	    }

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	}

}


/*
** delete the selected payee to our treeview and temp GList
*/
static void ui_tag_manage_dialog_delete(GtkWidget *widget, gpointer user_data)
{
struct ui_tag_manage_dialog_data *data;
Tag *item;
guint32 key;
gint result;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(ui_tag_manage_dialog) delete (data=%p)\n", data) );

	key = ui_tag_listview_get_selected_key(GTK_TREE_VIEW(data->LV_tag));
	if( key > 0 )
	{
	gchar *title;
	gchar *secondtext = NULL;

		item = da_tag_get(key);

		title = g_strdup_printf (
			_("Are you sure you want to permanently delete '%s'?"), item->name);

		if( item->usage_count > 0 )
		{
			secondtext = _("This payee is used.\n"
			    "Any transaction using that payee will be set to (no payee)");
		}

		result = ui_dialog_msg_confirm_alert(
				GTK_WINDOW(data->window),
				title,
				secondtext,
				_("_Delete")
			);

		g_free(title);

		if( result == GTK_RESPONSE_OK )
		{
			payee_move(key, 0);
			ui_tag_listview_remove_selected(GTK_TREE_VIEW(data->LV_tag));
			da_tag_remove(key);
			data->change++;
		}

	}
}


static void ui_tag_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_tag_manage_dialog_data *data;
gboolean sensitive;
guint32 key;

	DB( g_print("\n(ui_tag_manage_dialog) cursor changed\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	key = ui_tag_listview_get_selected_key(GTK_TREE_VIEW(data->LV_tag));

	sensitive = (key > 0) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->BT_edit, sensitive);
	gtk_widget_set_sensitive(data->BT_delete, sensitive);

}


/*
**
*/
static void ui_tag_manage_dialog_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_tag_manage_dialog_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

static void ui_tag_manage_dialog_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            user_data)
{
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("ui_tag_manage_dialog_onRowActivated()\n") );


	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	{
		ui_tag_manage_dialog_edit(GTK_WIDGET(treeview), NULL);
	}
}


GtkWidget *ui_tag_manage_dialog (void)
{
struct ui_tag_manage_dialog_data data;
GtkWidget *dialog, *content, *mainvbox, *box, *bbox, *treeview, *scrollwin, *table, *addreveal;
gint w, h, row;

	dialog = gtk_dialog_new_with_buttons (_("Manage Tags"),
					    GTK_WINDOW(GLOBALS->mainwindow),
						0,
					    _("_Close"), GTK_RESPONSE_ACCEPT,
					    NULL);

	/*dialog = g_object_new (GTK_TYPE_DIALOG, "use-header-bar", TRUE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Manage Tags"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW(GLOBALS->mainwindow));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	*/
	//gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	
	data.window = dialog;
	data.change = 0;

	//gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_TAG);

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(dialog), -1, h/PHI);

	
	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_tag_manage_dialog) dialog=%p, inst_data=%p\n", dialog, &data) );

    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);

	//dialog contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), SPACING_MEDIUM);

    //our table
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	row = 0;
	bbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (table), bbox, 0, row, 2, 1);
	//test headerbar
	//content = gtk_dialog_get_header_bar(GTK_DIALOG (dialog));

	row++;
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_grid_attach (GTK_GRID (table), box, 0, row, 2, 1);
	
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(box), scrollwin);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwin), HB_MINHEIGHT_LIST);
	gtk_widget_set_hexpand (scrollwin, TRUE);
	gtk_widget_set_vexpand (scrollwin, TRUE);
	treeview = ui_tag_listview_new(FALSE);
	data.LV_tag = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	row++;
	addreveal = gtk_revealer_new ();
	gtk_grid_attach (GTK_GRID (table), addreveal, 0, row, 2, 1);
	data.ST_name = gtk_entry_new ();
	gtk_entry_set_placeholder_text(GTK_ENTRY(data.ST_name), _("new tag") );
	gtk_widget_set_hexpand (data.ST_name, TRUE);
	gtk_container_add(GTK_CONTAINER(addreveal), data.ST_name);
	
	row++;
	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (table), bbox, 0, row, 2, 1);

	data.BT_add = gtk_toggle_button_new_with_mnemonic(_("_Add"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_add);

	//todo: useless ?
	data.BT_edit = gtk_button_new_with_mnemonic(_("_Edit"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_edit);

	data.BT_delete = gtk_button_new_with_mnemonic(_("_Delete"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_delete);

	
	//connect all our signals
	g_object_bind_property (data.BT_add, "active", addreveal, "reveal-child", G_BINDING_BIDIRECTIONAL);

	g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (ui_tag_manage_dialog_add), NULL);
	g_signal_connect(G_OBJECT(data.ST_name), "insert-text", G_CALLBACK(ui_tag_manage_filter_text_handler), NULL);
	
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_tag)), "changed", G_CALLBACK (ui_tag_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_tag), "row-activated", G_CALLBACK (ui_tag_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_edit), "clicked", G_CALLBACK (ui_tag_manage_dialog_edit), NULL);
	g_signal_connect (G_OBJECT (data.BT_delete), "clicked", G_CALLBACK (ui_tag_manage_dialog_delete), NULL);

	//setup, init and show dialog
	//tag_fill_usage();
	ui_tag_listview_populate(data.LV_tag, 0);
	ui_tag_manage_dialog_update(data.LV_tag, NULL);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

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

	gtk_widget_destroy (dialog);

	GLOBALS->changes_count += data.change;

	return NULL;
}

