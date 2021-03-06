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

#include "ui-payee.h"
#include "ui-category.h"

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


static GtkWidget *
container_get_nth(GtkBox *container, gint nth)
{
GList *lchild, *list;
GtkWidget *child;

	if(!GTK_IS_CONTAINER(container))
		return NULL;

	lchild = list = gtk_container_get_children (GTK_CONTAINER(container));
	child = g_list_nth_data (list, nth);
	g_list_free(lchild);
	
	return child;
}


GtkWidget *
ui_pay_entry_popover_get_entry(GtkBox *box)
{
	return container_get_nth(box, 0);
}


Payee
*ui_pay_entry_popover_get(GtkBox *box)
{
GtkWidget *entry;
gchar *name;
Payee *item = NULL;

	DB( g_print ("ui_pay_entry_popover_get()\n") );

	entry = container_get_nth(box, 0);
	if( entry != NULL && GTK_IS_ENTRY(entry) )
	{
		name = (gchar *)gtk_entry_get_text(GTK_ENTRY (entry));
		item = da_pay_get_by_name(name);
	}
	return item;
}


guint32
ui_pay_entry_popover_get_key_add_new(GtkBox *box)
{
Payee *item = ui_pay_entry_popover_get(box);
GtkWidget *entry;
GtkTreeModel *store;
	
	if( item == NULL )
	{
		/* automatic add */
		//todo: check prefs + ask the user here 1st time
		entry = container_get_nth(box, 0);
		if( entry != NULL && GTK_IS_ENTRY(entry) )
		{
			item = da_pay_malloc();
			item->name = g_strdup(gtk_entry_get_text(GTK_ENTRY (entry)));
			da_pay_append(item);

			store = gtk_entry_completion_get_model(gtk_entry_get_completion(GTK_ENTRY(entry)));
			if( store )
				gtk_list_store_insert_with_values(GTK_LIST_STORE(store), NULL, -1,
					0, item->name, -1);
		}
	}
	return item->key;
}


guint32
ui_pay_entry_popover_get_key(GtkBox *box)
{
Payee *item = ui_pay_entry_popover_get(box);

	return ((item != NULL) ? item->key : 0);
}


void
ui_pay_entry_popover_set_active(GtkBox *box, guint32 key)
{
GtkWidget *entry;

	DB( g_print ("ui_pay_comboboxentry_set_active()\n") );

	entry = container_get_nth(box, 0);
	if( entry != NULL && GTK_IS_ENTRY(entry) )
	{
	Payee *item = da_pay_get(key);

		ui_gtk_entry_set_text(GTK_WIDGET(entry), item != NULL ? item->name : "");
	}
}


static void 
ui_pay_entry_popover_cb_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
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
		gchar *item;

			gtk_tree_model_get(model, &iter, 0, &item, -1);
			gtk_entry_set_text(GTK_ENTRY(user_data), item);
			g_free(item);			
		}
	}
}


static void
ui_pay_entry_popover_populate(GtkListStore *store)
{
GHashTableIter hiter;
gpointer key, value;

	g_hash_table_iter_init (&hiter, GLOBALS->h_pay);
	while (g_hash_table_iter_next (&hiter, &key, &value))
	{
	Payee *pay = value;

		gtk_list_store_insert_with_values(GTK_LIST_STORE(store), NULL, -1,
			0, pay->name, -1);
	}

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

}


static void
ui_pay_entry_popover_function (GtkEditable *editable, gpointer user_data)
{

	DB( g_print("text changed to %s\n", gtk_entry_get_text(GTK_ENTRY(editable)) ) );
	


}


static void
ui_pay_entry_popover_cb_toggled (GtkToggleButton *togglebutton, gpointer user_data)
{
GtkWidget *entry = user_data;
GtkAllocation allocation;
GtkPopover *popover;

	if(GTK_IS_ENTRY(entry))
	{
		gtk_widget_get_allocation (entry, &allocation);
		popover = gtk_menu_button_get_popover(GTK_MENU_BUTTON(togglebutton));
		if(GTK_IS_POPOVER(popover))
		{
			gtk_widget_set_size_request (GTK_WIDGET(popover), allocation.width + (2*SPACING_POPOVER), -1);	
			DB( g_print("should set width to %d\n", allocation.width + (2*SPACING_POPOVER)) );	
		}
	}
}


static gint
ui_pay_entry_popover_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
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
ui_pay_entry_popover_destroy( GtkWidget *widget, gpointer user_data )
{

    DB( g_print ("[pay entry popover] destroy\n") );

}


GtkWidget *
ui_pay_entry_popover_new(GtkWidget *label)
{
GtkWidget *mainbox, *box, *entry, *menubutton, *image, *popover, *scrollwin, *treeview;
GtkListStore *store;
GtkEntryCompletion *completion;

    DB( g_print ("[pay entry popover] new\n") );

	mainbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET(mainbox)), GTK_STYLE_CLASS_LINKED);
	
	entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(mainbox), entry, TRUE, TRUE, 0);

	menubutton = gtk_menu_button_new ();
	//data->MB_template = menubutton;
	image = gtk_image_new_from_icon_name ("pan-down-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_container_add(GTK_CONTAINER(menubutton), image);
	gtk_menu_button_set_direction (GTK_MENU_BUTTON(menubutton), GTK_ARROW_LEFT );
	//gtk_widget_set_halign (menubutton, GTK_ALIGN_END);
	gtk_box_pack_start(GTK_BOX(mainbox), menubutton, FALSE, FALSE, 0);
	
    completion = gtk_entry_completion_new ();

	gtk_entry_set_completion (GTK_ENTRY (entry), completion);
	g_object_unref(completion);
	
	store = gtk_list_store_new (1,
		G_TYPE_STRING
		);

	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_pay_entry_popover_compare_func, NULL, NULL);

	ui_pay_entry_popover_populate(store);

	
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_entry_completion_set_text_column (completion, 0);

	gtk_widget_show_all(mainbox);


	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING_MEDIUM);
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_box_pack_start(GTK_BOX(box), scrollwin, TRUE, TRUE, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	//gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwin), HB_MINHEIGHT_LIST);
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(store));
	gtk_container_add(GTK_CONTAINER(scrollwin), GTK_WIDGET(treeview));
	gtk_widget_show_all(box);

	//gtk_widget_set_can_focus(GTK_WIDGET(treeview), FALSE);

GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
	
  renderer = gtk_cell_renderer_text_new ();

	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
		//taken from nemo, not exactly a resize to content, but good compromise
	    "width-chars", 40,
	    NULL);

	column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text",
                                                     0,
                                                     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(treeview), TRUE);

	
	//gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), GTK_SELECTION_BROWSE);
	
	
	//popover = create_popover (menubutton, box, GTK_POS_BOTTOM);
	popover = create_popover (menubutton, box, GTK_POS_LEFT);
	//gtk_widget_set_size_request (popover, HB_MINWIDTH_LIST, HB_MINHEIGHT_LIST);
	gtk_widget_set_vexpand(popover, TRUE);
	
	gtk_menu_button_set_popover(GTK_MENU_BUTTON(menubutton), popover);
	
	
	// connect our dispose function
	g_signal_connect (entry, "destroy", G_CALLBACK (ui_pay_entry_popover_destroy), NULL);

	g_signal_connect_after (entry  , "changed", G_CALLBACK (ui_pay_entry_popover_function), NULL);

	g_signal_connect (menubutton, "toggled", G_CALLBACK (ui_pay_entry_popover_cb_toggled), entry);
	
	g_signal_connect (treeview, "row-activated", G_CALLBACK (ui_pay_entry_popover_cb_row_activated), entry);

	#if( (GTK_MAJOR_VERSION == 3) && (GTK_MINOR_VERSION >= 22) )
		g_signal_connect_swapped(treeview, "row-activated", G_CALLBACK(gtk_popover_popdown), popover);
	#else
		g_signal_connect_swapped(treeview, "row-activated", G_CALLBACK(gtk_widget_hide), popover);
	#endif

	//g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK (ui_pay_entry_popover_cb_selection), entry);
	//g_signal_connect_swapped(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed", G_CALLBACK(gtk_popover_popdown), popover);
	
	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);

	//gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_LIST, -1);

	return mainbox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


/**
 * ui_pay_comboboxentry_add:
 *
 * Add a single element (useful for dynamics add)
 *
 * Return value: --
 *
 */
static void
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

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
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

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	item = da_pay_get_by_name(name);

	if( item != NULL )
		return item->key;

	return 0;
}


Payee
*ui_pay_comboboxentry_get(GtkComboBox *entry_box)
{
gchar *name;
Payee *item = NULL;

	DB( g_print ("ui_pay_comboboxentry_get()\n") );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	item = da_pay_get_by_name(name);

	return item;
}


gboolean
ui_pay_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key)
{
Payee *item;

	DB( g_print ("ui_pay_comboboxentry_set_active()\n") );

	DB( g_print("- key:%d\n", key) );
	
	if( key > 0 )
	{
		item = da_pay_get(key);
		if( item != NULL )
		{
			DB( g_print("- set combo to '%s'\n", item->name) );

			gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), item->name);
			return TRUE;
		}
	}

	DB( g_print("- set combo to ''\n") );
	
	gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), "");
	return FALSE;
}


static void
ui_pay_comboboxentry_populate_ghfunc(gpointer key, gpointer value, struct payPopContext *ctx)
{
GtkTreeIter  iter;
Payee *pay = value;

	if( ( pay->key != ctx->except_key ) )
	{
		//gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		//gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter, 0, pay->name, -1);
		gtk_list_store_insert_with_values(GTK_LIST_STORE(ctx->model), &iter, -1,
			0, pay->name, -1);
	}
}


//not used except here
static void
ui_pay_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key)
{
GtkTreeModel *model;
//GtkEntryCompletion *completion;
struct payPopContext ctx;

    DB( g_print ("ui_pay_comboboxentry_populate\n") );

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));
	//completion = gtk_entry_get_completion(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));

	/* keep our model alive and detach from comboboxentry and completion */
	//g_object_ref(model);
	//gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), NULL);
	//gtk_entry_completion_set_model (completion, NULL);

	/* clear and populate */
	ctx.model = model;
	ctx.except_key = except_key;
	gtk_list_store_clear (GTK_LIST_STORE(model));

	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(GTK_LIST_STORE(model)), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	g_hash_table_foreach(hash, (GHFunc)ui_pay_comboboxentry_populate_ghfunc, &ctx);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	/* reatach our model */
	//g_print("reattach\n");
	//gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	//gtk_entry_completion_set_model (completion, model);
	//g_object_unref(model);
	
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

	//leak
	g_free(name);

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

    DB( g_print ("ui_pay_comboboxentry_new()\n") );

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

	//gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_LIST, -1);

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
gint sortcol = GPOINTER_TO_INT(userdata);
Payee *entry1, *entry2;
gint retval = 0;

	gtk_tree_model_get(model, a, LST_DEFPAY_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFPAY_DATAS, &entry2, -1);

    switch (sortcol)
    {
		case LST_DEFPAY_SORT_NAME:
			retval = hb_string_utf8_compare(entry1->name, entry2->name);
			break;
		case LST_DEFPAY_SORT_USED:
			retval = entry1->usage_count - entry2->usage_count;
			break;
		case LST_DEFPAY_SORT_DEFPAY:
			retval = entry1->paymode - entry2->paymode;
			break;
		case LST_DEFPAY_SORT_DEFCAT:
			{
			Category *c1, *c2;

				c1 = da_cat_get(entry1->kcat);
				c2 = da_cat_get(entry2->kcat);
				if( c1 != NULL && c2 != NULL )
				{
					retval = hb_string_utf8_compare(c1->fullname, c2->fullname);
				}
			}
			break;
		default:
			g_return_val_if_reached(0);
	}
		
    return retval;
}


static void
ui_pay_listview_count_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Payee *entry;
gchar buffer[256];

	gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &entry, -1);
	if(entry->usage_count > 0)
	{
		g_snprintf(buffer, 256-1, "%d", entry->usage_count);
		g_object_set(renderer, "text", buffer, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);
}


static void
ui_pay_listview_defcat_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Payee *entry;
Category *cat;

	gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &entry, -1);

	cat = da_cat_get(entry->kcat);
	if( cat != NULL )
	{
		g_object_set(renderer, "text", cat->fullname, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);
}

static void
ui_pay_listview_info_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Payee *entry;

	gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &entry, -1);

	g_object_set(renderer, "icon-name", get_paymode_icon_name(entry->paymode), NULL);

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
		string = g_strdup_printf ("%d > %s [ft=%d]", entry->key, name, entry->flt_select);
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
GtkTreeModel *model;
GtkTreeIter	iter;

	if( item->name != NULL )
	{
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

	DB( g_print("ui_pay_listview_remove_selected() \n") );

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
}


struct PayListContext {
	GtkTreeModel *model;
	gchar *needle;
};


static void ui_pay_listview_populate_ghfunc(gpointer key, gpointer value, struct PayListContext *context)
{
GtkTreeIter	iter;
Payee *item = value;
gboolean hastext = FALSE;
gboolean insert = TRUE;
	
	//DB( g_print(" populate: %p\n", key) );
	if( context->needle != NULL )
		hastext = (strlen(context->needle) >= 2) ? TRUE : FALSE;

	if(hastext)
	{
		insert = hb_string_utf8_strstr(item->name, context->needle, FALSE);
	}

	if( insert == TRUE)
	{
		gtk_list_store_insert_with_values(GTK_LIST_STORE(context->model), &iter, -1,
			LST_DEFPAY_TOGGLE	, FALSE,
			LST_DEFPAY_DATAS, item,
			-1);
	}

}


void ui_pay_listview_populate(GtkWidget *treeview, gchar *needle)
{
struct PayListContext context;
	
	DB( g_print("ui_pay_listview_populate \n") );

	context.model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	context.needle = needle;
	
	gtk_list_store_clear (GTK_LIST_STORE(context.model));

	//g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	//gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	g_hash_table_foreach(GLOBALS->h_pay, (GHFunc)ui_pay_listview_populate_ghfunc, &context);

	//gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	//g_object_unref(model);
}


static gboolean ui_pay_listview_search_equal_func (GtkTreeModel *model,
                               gint column,
                               const gchar *key,
                               GtkTreeIter *iter,
                               gpointer search_data)
{
  gboolean retval = TRUE;
  gchar *normalized_string;
  gchar *normalized_key;
  gchar *case_normalized_string = NULL;
  gchar *case_normalized_key = NULL;
  Payee *item;
	
  //gtk_tree_model_get_value (model, iter, column, &value);
  gtk_tree_model_get(model, iter, LST_DEFPAY_DATAS, &item, -1);

  if(item !=  NULL)
  {
	  normalized_string = g_utf8_normalize (item->name, -1, G_NORMALIZE_ALL);
	  normalized_key = g_utf8_normalize (key, -1, G_NORMALIZE_ALL);

	  if (normalized_string && normalized_key)
		{
		  case_normalized_string = g_utf8_casefold (normalized_string, -1);
		  case_normalized_key = g_utf8_casefold (normalized_key, -1);

		  if (strncmp (case_normalized_key, case_normalized_string, strlen (case_normalized_key)) == 0)
		    retval = FALSE;
		}

	  g_free (normalized_key);
	  g_free (normalized_string);
	  g_free (case_normalized_key);
	  g_free (case_normalized_string);
  }
  return retval;
}


GtkWidget *
ui_pay_listview_new(gboolean withtoggle, gboolean withcount)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	DB( g_print("ui_pay_listview_new() \n") );

	store = gtk_list_store_new(
		NUM_LST_DEFPAY,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER
		);

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);

	// column: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
						renderer, "active", LST_DEFPAY_TOGGLE, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (G_OBJECT(renderer), "toggled",
			    G_CALLBACK (ui_pay_listview_toggled_cb), store);

	}

	// column: name
	renderer = gtk_cell_renderer_text_new ();

	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
		//taken from nemo, not exactly a resize to content, but good compromise
	    "width-chars", 40,
	    NULL);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_pay_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFPAY_DATAS), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST);
	gtk_tree_view_column_set_sort_column_id (column, LST_DEFPAY_SORT_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column: usage
	if( withcount == TRUE )
	{
		renderer = gtk_cell_renderer_text_new ();
		g_object_set(renderer, "xalign", 0.5, NULL);

		column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column, _("Usage"));
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ui_pay_listview_count_cell_data_function, GINT_TO_POINTER(LST_DEFPAY_DATAS), NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, LST_DEFPAY_SORT_USED);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	}

	// column: payment
	if( withtoggle == FALSE )
	{
		column = gtk_tree_view_column_new();
		//TRANSLATORS: this is abbreviation for Payment
		gtk_tree_view_column_set_title(column, _("Pay."));
		renderer = gtk_cell_renderer_pixbuf_new ();
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ui_pay_listview_info_cell_data_function, NULL, NULL);
		gtk_tree_view_column_set_sort_column_id (column, LST_DEFPAY_SORT_DEFPAY);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	}	

	// column: category
	if( withtoggle == FALSE )
	{
		renderer = gtk_cell_renderer_text_new ();
		g_object_set(renderer, 
			"ellipsize", PANGO_ELLIPSIZE_END,
			"ellipsize-set", TRUE,
			//taken from nemo, not exactly a resize to content, but good compromise
			"width-chars", 40,
			NULL);

		column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column, _("Category"));
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ui_pay_listview_defcat_cell_data_function, GINT_TO_POINTER(LST_DEFPAY_DATAS), NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_column_set_sort_column_id (column, LST_DEFPAY_SORT_DEFCAT);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	}


	/* empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(treeview), ui_pay_listview_search_equal_func, NULL, NULL);

	// treeview attribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), withcount);

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_SORT_NAME, ui_pay_listview_compare_func, GINT_TO_POINTER(LST_DEFPAY_SORT_NAME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_SORT_USED, ui_pay_listview_compare_func, GINT_TO_POINTER(LST_DEFPAY_SORT_USED), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_SORT_DEFPAY, ui_pay_listview_compare_func, GINT_TO_POINTER(LST_DEFPAY_SORT_DEFPAY), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFPAY_SORT_DEFCAT, ui_pay_listview_compare_func, GINT_TO_POINTER(LST_DEFPAY_SORT_DEFCAT), NULL);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFPAY_SORT_NAME, GTK_SORT_ASCENDING);

	return treeview;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void
ui_pay_manage_dialog_delete_unused( GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data = user_data;
gboolean result;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_pay_manage_dialog) delete unused - data %p\n", data) );

	result = ui_dialog_msg_confirm_alert(
			GTK_WINDOW(data->window),
			_("Delete unused payee"),
			_("Are you sure you want to\npermanently delete unused payee?"),
			_("_Delete")
		);

	if( result == GTK_RESPONSE_OK )
	{
	GtkTreeModel *model;	
		
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
		gtk_list_store_clear (GTK_LIST_STORE(model));
		
		payee_delete_unused();
	
		ui_pay_listview_populate (data->LV_pay, NULL);
	}
}


/**
 * ui_pay_manage_dialog_load_csv:
 *
 */
static void
ui_pay_manage_dialog_load_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data = user_data;
gchar *filename = NULL;
gchar *error;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_pay_manage_dialog) load csv - data %p\n", data) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		if( !payee_load_csv(filename, &error) )
		{
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The CSV file must contains the exact numbers of column,\nseparated by a semi-colon, please see the help for more details.")
					);
		}

		g_free( filename );
		ui_pay_listview_populate(data->LV_pay, NULL);
	}
}

/**
 * ui_pay_manage_dialog_save_csv:
 *
 */
static void
ui_pay_manage_dialog_save_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data = user_data;
gchar *filename = NULL;

	DB( g_print("(ui_pay_manage_dialog) save csv\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

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

	item = da_pay_malloc ();
	item->name = g_strdup(name);

	g_strstrip(item->name);
	
	if( strlen(item->name) > 0 )
	{
		if( da_pay_append(item) )
		{
			ui_pay_listview_add(GTK_TREE_VIEW(data->LV_pay), item);
			data->change++;
		}
	}
	else
		da_pay_free (item);
		
	gtk_entry_set_text(GTK_ENTRY(data->ST_name), "");
}


static void ui_pay_manage_dialog_edit_entry_cb(GtkEditable *editable, gpointer user_data)
{
GtkDialog *window = user_data;
const gchar *buffer;

	buffer = gtk_entry_get_text(GTK_ENTRY(editable));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


static void ui_pay_manage_dialog_edit(GtkWidget *dowidget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget;
GtkWidget *ST_name, *PO_cat, *NU_mode;
gint crow, row;
guint32 key;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(dowidget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) modify %p\n", data) );

	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));
	if( key > 0 )
	{
	Payee *item;

		item = da_pay_get( key );

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

		// group :: Default
		group_grid = gtk_grid_new ();
		gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
		gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
		gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
		label = make_label_group(_("Default"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 4, 1);

		row = 1;
		label = make_label_widget(_("Pa_yment:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		//5.2.4 we drop internal xfer here as it will disapear 
		//widget = make_paymode_nointxfer(label);
		widget = make_paymode_nointxfer(label);
		NU_mode = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

		row++;
		label = make_label_widget(_("_Category:"));
		gtk_grid_attach (GTK_GRID (group_grid), label, 0, row, 1, 1);
		widget = ui_cat_comboboxentry_new(label);
		PO_cat = widget;
		gtk_widget_set_hexpand (widget, TRUE);
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 1, 1);

		//setup
		gtk_entry_set_text(GTK_ENTRY(ST_name), item->name);
		gtk_widget_grab_focus (ST_name);
		gtk_entry_set_activates_default (GTK_ENTRY(ST_name), TRUE);
		
		ui_cat_comboboxentry_populate(GTK_COMBO_BOX(PO_cat), GLOBALS->h_cat);
		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(PO_cat), item->kcat);

		gtk_combo_box_set_active(GTK_COMBO_BOX(NU_mode), item->paymode);

		g_signal_connect (G_OBJECT (ST_name), "changed", G_CALLBACK (ui_pay_manage_dialog_edit_entry_cb), dialog);

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
				if( payee_rename(item, name) )
				{
					//to redraw the active entry
					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_pay));
					data->change++;
				}
				else
				{
					ui_dialog_msg_infoerror(GTK_WINDOW(dialog), GTK_MESSAGE_ERROR,
						_("Error"),
						_("Cannot rename this Payee,\n"
						"from '%s' to '%s',\n"
						"this name already exists."),
						item->name,
						name
						);

				}
			}

			item->kcat    = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(PO_cat));
			item->paymode = gtk_combo_box_get_active(GTK_COMBO_BOX(NU_mode));

	    }

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	}

}


static void ui_pay_manage_dialog_merge_entry_cb(GtkComboBox *widget, gpointer user_data)
{
GtkDialog *window = user_data;
gchar *buffer;

	buffer = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (widget))));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_OK, strlen(buffer) > 0 ? TRUE : FALSE);
}


static void ui_pay_manage_dialog_merge(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
GtkWidget *dialog, *content, *mainvbox;
GtkWidget *getwidget, *togglebutton;
GtkTreeSelection *selection;
GtkTreeModel *model;
GtkTreeIter iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defayee) merge %p\n", data) );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_pay));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Payee *srcpay;
	gchar *title;
	gchar *secondtext;

		gtk_tree_model_get(model, &iter, LST_DEFPAY_DATAS, &srcpay, -1);

		title = g_strdup_printf (
			_("Merge payee '%s'"), srcpay->name);
		
		dialog = gtk_message_dialog_new (GTK_WINDOW (data->window),
			                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			                              GTK_MESSAGE_WARNING,
			                              GTK_BUTTONS_NONE,
			                              title,
		                             	  NULL
			                              );

		gtk_dialog_add_buttons (GTK_DIALOG(dialog),
				_("_Cancel"), GTK_RESPONSE_CANCEL,
				_("Merge"), GTK_RESPONSE_OK,
				NULL);

		gtk_dialog_set_default_response(GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);

		content = gtk_message_dialog_get_message_area(GTK_MESSAGE_DIALOG (dialog));
		mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);

		secondtext = _("Transactions assigned to this payee,\n"
			  "will be moved to the payee selected below.");

		g_object_set(GTK_MESSAGE_DIALOG (dialog), "secondary-text", secondtext, NULL);
		g_free(title);

		getwidget = ui_pay_comboboxentry_new(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, FALSE, FALSE, 0);

		secondtext = g_strdup_printf (
			_("_Delete the payee '%s'"), srcpay->name);
		togglebutton = gtk_check_button_new_with_mnemonic(secondtext);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton), TRUE);
		g_free(secondtext);
		gtk_box_pack_start (GTK_BOX (mainvbox), togglebutton, FALSE, FALSE, 0);

		//setup 
		//gtk_combo_box_set_active(GTK_COMBO_BOX(getwidget), oldpos);
		g_signal_connect (G_OBJECT (getwidget), "changed", G_CALLBACK (ui_pay_manage_dialog_merge_entry_cb), dialog);
		gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, FALSE);

		ui_pay_comboboxentry_populate_except(GTK_COMBO_BOX(getwidget), GLOBALS->h_pay, srcpay->key);
		gtk_widget_grab_focus (getwidget);

		gtk_widget_show_all(mainvbox);
		
		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_OK)
		{
		GtkTreeModel *model;
		Payee *newpay;
		guint dstpaykey;

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_pay));
			gtk_list_store_clear (GTK_LIST_STORE(model));

			dstpaykey = ui_pay_comboboxentry_get_key_add_new(GTK_COMBO_BOX(getwidget));

			payee_move(srcpay->key, dstpaykey);

			newpay = da_pay_get(dstpaykey);

			//#1771720: update count
			newpay->usage_count += srcpay->usage_count;
			srcpay->usage_count = 0;

			// add the new payee to listview
			if(newpay)
				ui_pay_listview_add(GTK_TREE_VIEW(data->LV_pay), newpay);

			// delete the old payee
			if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton)) )
			{
				DB( g_print(" -> delete %d '%s'\n", srcpay->key, srcpay->name ) );

				da_pay_remove(srcpay->key);
				ui_pay_listview_remove_selected(GTK_TREE_VIEW(data->LV_pay));
			}


			data->change++;

			ui_pay_listview_populate(data->LV_pay, NULL);
		}

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	}

}


/*
** delete the selected payee to our treeview and temp GList
*/
static void ui_pay_manage_dialog_delete(GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
Payee *item;
guint32 key;
gint result;


	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(ui_pay_manage_dialog) delete (data=%p)\n", data) );

	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));
	if( key > 0 )
	{
	gchar *title;
	gchar *secondtext = NULL;

		item = da_pay_get(key);

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
			ui_pay_listview_remove_selected(GTK_TREE_VIEW(data->LV_pay));
			da_pay_remove(key);
			data->change++;
		}

	}
}


static void ui_pay_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data;
gboolean sensitive;
guint32 key;

	DB( g_print("\n(ui_pay_manage_dialog) cursor changed\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	key = ui_pay_listview_get_selected_key(GTK_TREE_VIEW(data->LV_pay));

	sensitive = (key > 0) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->BT_edit, sensitive);
	gtk_widget_set_sensitive(data->BT_merge, sensitive);
	gtk_widget_set_sensitive(data->BT_delete, sensitive);

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
                       gpointer            user_data)
{
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("ui_pay_manage_dialog_onRowActivated()\n") );


	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	{
		ui_pay_manage_dialog_edit(GTK_WIDGET(treeview), NULL);
	}
}


static void
ui_pay_manage_search_changed_cb (GtkWidget *widget, gpointer user_data)
{
struct ui_pay_manage_dialog_data *data = user_data;
gchar *needle;

	DB( g_printf("\n[ui_pay_manage_dialog] search_changed_cb\n") );

	needle = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_search));
	ui_pay_listview_populate(data->LV_pay, needle);
}


GtkWidget *ui_pay_manage_dialog (void)
{
struct ui_pay_manage_dialog_data data;
GtkWidget *dialog, *content, *mainvbox, *box, *bbox, *treeview, *scrollwin, *table;
GtkWidget *menu, *menuitem, *widget, *image, *searchbar, *addreveal;
gint w, h, row;

	dialog = gtk_dialog_new_with_buttons (_("Manage Payees"),
					    GTK_WINDOW(GLOBALS->mainwindow),
						0,
					    _("_Close"), GTK_RESPONSE_ACCEPT,
					    NULL);

	/*dialog = g_object_new (GTK_TYPE_DIALOG, "use-header-bar", TRUE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Manage Payees"));
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW(GLOBALS->mainwindow));
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	*/
	//gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	
	data.window = dialog;
	data.change = 0;

	gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_PAYEE);

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(dialog), -1, h/PHI);

	
	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_pay_manage_dialog) dialog=%p, inst_data=%p\n", dialog, &data) );

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

	menu = gtk_menu_new ();
	gtk_widget_set_halign (menu, GTK_ALIGN_END);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Import CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_pay_manage_dialog_load_csv), &data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("E_xport CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_pay_manage_dialog_save_csv), &data);
	
	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	
	menuitem = gtk_menu_item_new_with_mnemonic (_("_Delete unused"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_pay_manage_dialog_delete_unused), &data);
	
	gtk_widget_show_all (menu);

	
	widget = gtk_menu_button_new();
	image = gtk_image_new_from_icon_name (ICONNAME_HB_BUTTON_MENU, GTK_ICON_SIZE_MENU);
	g_object_set (widget, "image", image, "popup", GTK_MENU(menu),  NULL);
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	gtk_box_pack_end(GTK_BOX (bbox), widget, FALSE, FALSE, 0);
	//gtk_header_bar_pack_end(GTK_HEADER_BAR (content), widget);
	
	data.BT_search = gtk_toggle_button_new ();
	image = gtk_image_new_from_icon_name (ICONNAME_SYSTEM_SEARCH, GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image (GTK_BUTTON(data.BT_search), image);
	gtk_box_pack_end(GTK_BOX (bbox), data.BT_search, FALSE, FALSE, 0);
	//gtk_header_bar_pack_end(GTK_HEADER_BAR (content), data.BT_search);
	
	//search + list
	row++;
	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_grid_attach (GTK_GRID (table), box, 0, row, 2, 1);

	searchbar = gtk_search_bar_new();
	gtk_container_add(GTK_CONTAINER(box), searchbar);
	widget = make_search();
	data.ST_search = widget;
	gtk_container_add (GTK_CONTAINER (searchbar), widget);
	gtk_search_bar_connect_entry(GTK_SEARCH_BAR(searchbar), GTK_ENTRY(data.ST_search));
	gtk_search_bar_set_show_close_button(GTK_SEARCH_BAR(searchbar), TRUE);
	
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(box), scrollwin);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwin), HB_MINHEIGHT_LIST);
	gtk_widget_set_hexpand (scrollwin, TRUE);
	gtk_widget_set_vexpand (scrollwin, TRUE);
	treeview = ui_pay_listview_new(FALSE, TRUE);
	data.LV_pay = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	row++;
	addreveal = gtk_revealer_new ();
	gtk_grid_attach (GTK_GRID (table), addreveal, 0, row, 2, 1);
	data.ST_name = gtk_entry_new ();
	gtk_entry_set_placeholder_text(GTK_ENTRY(data.ST_name), _("new payee") );
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

	data.BT_merge = gtk_button_new_with_mnemonic(_("_Merge"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_merge);

	data.BT_delete = gtk_button_new_with_mnemonic(_("_Delete"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_delete);

	
	//connect all our signals
	g_object_bind_property (data.BT_add, "active", addreveal, "reveal-child", G_BINDING_BIDIRECTIONAL);
	g_object_bind_property (data.BT_search, "active", searchbar, "search-mode-enabled", G_BINDING_BIDIRECTIONAL);

	g_signal_connect (G_OBJECT (data.ST_search), "search-changed", G_CALLBACK (ui_pay_manage_search_changed_cb), &data);
	g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (ui_pay_manage_dialog_add), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_pay)), "changed", G_CALLBACK (ui_pay_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_pay), "row-activated", G_CALLBACK (ui_pay_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_edit), "clicked", G_CALLBACK (ui_pay_manage_dialog_edit), NULL);
	g_signal_connect (G_OBJECT (data.BT_merge), "clicked", G_CALLBACK (ui_pay_manage_dialog_merge), NULL);
	g_signal_connect (G_OBJECT (data.BT_delete), "clicked", G_CALLBACK (ui_pay_manage_dialog_delete), NULL);

	//setup, init and show dialog
	payee_fill_usage();
	ui_pay_listview_populate(data.LV_pay, NULL);
	ui_pay_manage_dialog_update(data.LV_pay, NULL);

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

