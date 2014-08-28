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

#include "ui-category.h"

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
 * ui_cat_comboboxentry_get_name:
 *
 * get the name of the active category or -1
 *
 * Return value: a new allocated name tobe freed with g_free
 *
 */
gchar *
ui_cat_comboboxentry_get_name(GtkComboBox *entry_box)
{
gchar *cbname;
gchar *name = NULL;

    DB( g_print ("ui_cat_comboboxentry_get_name()\n") );

	cbname = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	if( cbname != NULL)
	{
		name = g_strdup(cbname);
		g_strstrip(name);
	}

	return name;
}


/**
 * ui_cat_comboboxentry_get_key:
 *
 * get the key of the active category or -1
 *
 * Return value: the key or -1
 *
 */
guint32
ui_cat_comboboxentry_get_key_add_new(GtkComboBox *entry_box)
{
Category *item;
gchar *name;

	DB( g_print ("ui_cat_comboboxentry_get_key()\n") );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));

	if( name == NULL)
		return -1;

	item = da_cat_get_by_fullname(name);
	if(item == NULL)
	{
		/* automatic add */
		//todo: check prefs + ask the user here 1st time
		item = da_cat_append_ifnew_by_fullname(name, FALSE);

		ui_cat_comboboxentry_add(entry_box, item);
	}

	return item->key;
}


/**
 * ui_cat_comboboxentry_get_key:
 *
 * get the key of the active category or -1
 *
 * Return value: the key or -1
 *
 */
guint32
ui_cat_comboboxentry_get_key(GtkComboBox *entry_box)
{
Category *item;
gchar *name;

	DB( g_print ("ui_cat_comboboxentry_get_key()\n") );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	if( name == NULL)
		return -1;

	item = da_cat_get_by_fullname(name);
	if(item != NULL)
		return item->key;

	return -1;
}

gboolean
ui_cat_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key)
{
Category *item;
gchar *fullname;

    DB( g_print ("ui_cat_comboboxentry_set_active()\n") );


	if( key > 0 )
	{
		item = da_cat_get(key);
		if( item != NULL)
		{
			if( item->parent == 0)
				gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), item->name);
			else
			{
				fullname = da_cat_get_fullname(item);
				gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), fullname);
				g_free(fullname);
			}
			return TRUE;
		}
	}
	gtk_entry_set_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))), "");
	return FALSE;
}

/**
 * ui_cat_comboboxentry_add:
 *
 * Add a single element (useful for dynamics add)
 *
 * Return value: --
 *
 */
void
ui_cat_comboboxentry_add(GtkComboBox *entry_box, Category *item)
{

    DB( g_print ("ui_cat_comboboxentry_add()\n") );


    DB( g_print (" -> try to add: '%s'\n", item->name) );

	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter  iter;

gchar *fullname, *name;

		fullname = da_cat_get_fullname(item);
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));

		if( item->parent == 0 )
			name = g_strdup(item->name);
		else
			name = g_strdup_printf(" - %s", item->name);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, fullname,
			1, name,
			-1);

	g_free(fullname);
	g_free(name);

	}
}


static void
ui_cat_comboboxentry_populate_ghfunc(gpointer key, gpointer value, struct catPopContext *ctx)
{
GtkTreeIter  iter;
Category *item = value;
gchar *fullname, *name;
gchar type;

	if( ( item->key != ctx->except_key ) )
	{
		type = (item->flags & GF_INCOME) ? '+' : '-';

		fullname = da_cat_get_fullname(item);

		//DB( g_print ("cat combo populate [%d:%d] %s\n", item->parent, item->key, fullname) );

		if(item->key == 0)
			name = g_strdup(item->name);
		else
		{
			if( item->parent == 0 )
				name = g_strdup_printf("%s [%c]", item->name, type);
			else
				name = g_strdup_printf(" %c %s", type, item->name);
		}
		
		gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter,
			0, fullname,
			1, name,
		    2, item->parent == 0 ? 1 : 0,
			-1);

		g_free(fullname);
		g_free(name);
	}

}

/**
 * ui_cat_comboboxentry_populate:
 *
 * Populate the list and completion
 *
 * Return value: --
 *
 */
void
ui_cat_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash)
{
	ui_cat_comboboxentry_populate_except(entry_box, hash, -1);
}

void
ui_cat_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key)
{
GtkTreeModel *model;
GtkEntryCompletion *completion;
struct catPopContext ctx;

    DB( g_print ("ui_cat_comboboxentry_populate()\n") );

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
	g_hash_table_foreach(hash, (GHFunc)ui_cat_comboboxentry_populate_ghfunc, &ctx);

	/* reatach our model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	gtk_entry_completion_set_model (completion, model);
	g_object_unref(model);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

}



static gint
ui_cat_comboboxentry_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint retval = 0;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, 0, &name1, -1);
    gtk_tree_model_get(model, b, 0, &name2, -1);

	retval = hb_string_utf8_compare(name1, name2);
/*
	retval = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
	if(!retval)
	{
		retval = hb_string_utf8_compare(entry1->name, entry2->name);
	}
*/
    g_free(name2);
    g_free(name1);

  	return retval;
  }



static void
ui_cat_comboboxentry_test (GtkCellLayout   *cell_layout,
		   GtkCellRenderer *cell,
		   GtkTreeModel    *tree_model,
		   GtkTreeIter     *iter,
		   gpointer         data)
{
gchar *name;
gboolean subcat;
gint style;

//PANGO_STYLE_ITALIC
	
	gtk_tree_model_get(tree_model, iter,
		1, &name,
	    2, &subcat,
		-1);

	style = subcat == 0 ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;

	if( name == NULL )
		name = _("(no category)");  //todo: not used
	
	g_object_set(cell,
	    "style", style,
		"text", name,
		NULL);
}



static gboolean
ui_cat_comboboxentry_completion_func (GtkEntryCompletion *completion,
                                              const gchar        *key,
                                              GtkTreeIter        *iter,
                                              gpointer            user_data)
{
  gchar *item = NULL;
  gchar *normalized_string;
  gchar *case_normalized_string;

  gboolean ret = FALSE;

  GtkTreeModel *model;

  model = gtk_entry_completion_get_model (completion);

  gtk_tree_model_get (model, iter,
                      0, &item,
                      -1);

  if (item != NULL)
    {
      normalized_string = g_utf8_normalize (item, -1, G_NORMALIZE_ALL);

      if (normalized_string != NULL)
        {
          case_normalized_string = g_utf8_casefold (normalized_string, -1);

			//g_print("match '%s' for '%s' ?\n", key, case_normalized_string);
		//if (!strncmp (key, case_normalized_string, strlen (key)))
          if (g_strstr_len (case_normalized_string, strlen (case_normalized_string), key ))
			{
        		ret = TRUE;
				//	g_print(" ==> yes !\n");
				
			}
				
          g_free (case_normalized_string);
        }
      g_free (normalized_string);
    }
  g_free (item);

  return ret;
}


/**
 * ui_cat_comboboxentry_new:
 *
 * Create a new category comboboxentry
 *
 * Return value: the new widget
 *
 */
GtkWidget *
ui_cat_comboboxentry_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *comboboxentry;
GtkEntryCompletion *completion;
GtkCellRenderer    *renderer;

    DB( g_print ("ui_cat_comboboxentry_new()\n") );


	store = gtk_list_store_new (3,
		G_TYPE_STRING, 			//fullname Car:Fuel
		G_TYPE_STRING, 			//name	Car or Fuel
	    G_TYPE_BOOLEAN			//subcat = 1
		);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_cat_comboboxentry_compare_func, NULL, NULL);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
	g_object_set(completion, "text-column", 0, NULL);
	gtk_entry_completion_set_match_func(completion, ui_cat_comboboxentry_completion_func, NULL, NULL);
	gtk_entry_completion_set_minimum_key_length(completion, 2);

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (completion), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (completion), renderer, "text", 0, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (completion),
					    renderer,
					    ui_cat_comboboxentry_test,
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
					    ui_cat_comboboxentry_test,
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
ui_cat_listview_fixed_toggled (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFCAT_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, LST_DEFCAT_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

/*
**
** The function should return:
** a negative integer if the first value comes before the second,
** 0 if they are equal,
** or a positive integer if the first value comes after the second.
*/
static gint
ui_cat_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint retval = 0;
Category *entry1, *entry2;

	gtk_tree_model_get(model, a, LST_DEFCAT_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFCAT_DATAS, &entry2, -1);

	retval = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
	if(!retval)
	{
		retval = hb_string_utf8_compare(entry1->name, entry2->name);
	}
    return retval;
}


/*
** draw some text from the stored data structure
*/
static void
ui_cat_listview_text_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar *name;
gchar *string;

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);
	if(entry->key == 0)
		name = _("(no category)");
	else
		name = entry->name;

	gchar type = (entry->flags & GF_INCOME) ? '+' : '-';

	#if MYDEBUG
	string = g_markup_printf_escaped ("%d > [%d] %s [%c] %d", entry->key, entry->parent, name, type, entry->flags );
	#else
	if(entry->key == 0)
		string = g_strdup(name);
	else
	{
		if( entry->parent == 0 )
			string = g_markup_printf_escaped("%s [%c]", name, type);
		else
			string = g_markup_printf_escaped(" %c <i>%s</i>", type, name);
			//string = g_strdup_printf(" - %s", name);
	}
	#endif

	//g_object_set(renderer, "text", string, NULL);
	g_object_set(renderer, "markup", string, NULL);

	g_free(string);

}




void
ui_cat_listview_add(GtkTreeView *treeview, Category *item, GtkTreeIter	*parent)
{
GtkTreeModel *model;
GtkTreeIter	iter;
GtkTreePath *path;

	DB( g_print ("ui_cat_listview_add()\n") );

	if( item->name != NULL )
	{
		model = gtk_tree_view_get_model(treeview);

		gtk_tree_store_append (GTK_TREE_STORE(model), &iter, parent);
		gtk_tree_store_set (GTK_TREE_STORE(model), &iter,
			LST_DEFCAT_TOGGLE, FALSE,
			LST_DEFCAT_DATAS, item,
			LST_DEFCAT_NAME, item->name,
			-1);

		//select the added line

		path = gtk_tree_model_get_path(model, &iter);
		gtk_tree_view_expand_to_path (treeview, path);
		gtk_tree_path_free(path);
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(treeview), &iter);
	}

}

Category *
ui_cat_listview_get_selected(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print ("ui_cat_listview_get_selected()\n") );

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);
		if( item->key != 0 )
			return item;
	}
	return NULL;
}

Category *
ui_cat_listview_get_selected_parent(GtkTreeView *treeview, GtkTreeIter *return_iter)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
GtkTreePath *path;
Category *item;

	DB( g_print ("ui_cat_listview_get_selected_parent()\n") );


	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		path = gtk_tree_model_get_path(model, &iter);

		DB( g_print ("path depth = %d\n", gtk_tree_path_get_depth(path)) );


		if(gtk_tree_path_get_depth(path) > 1)
		{
			if( gtk_tree_path_up(path) )
			{

				DB( g_print ("up ok\n") );

				if(gtk_tree_model_get_iter(model, &iter, path))
				{

					DB( g_print ("iter ok\n") );


					gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);
					if( item->key != 0 )
					{
						*return_iter = iter;
						return item;
					}
				}
			}
		}
		else
		{

			DB( g_print ("path <=1\n") );

					gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

					if( item->key != 0 )
					{
						*return_iter = iter;
						return item;
					}


		}
	}
	return NULL;
}

gboolean ui_cat_listview_remove (GtkTreeModel *model, guint32 key)
{
GtkTreeIter  iter, child;
gboolean     valid, cvalid;
Category *item;

	DB( g_print("ui_cat_listview_remove() \n") );

    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
    while (valid)
    {
		gtk_tree_model_get (model, &iter, LST_DEFCAT_DATAS, &item, -1);

		DB( g_print(" + item %p, %s\n", item, item->name) );

		if(item->key == key || item->parent == key)
		{
			DB( g_print(" + removing cat %s\n", item->name) );
			gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
		}

		// iter children
		cvalid = gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter);
		while(cvalid)
		{
			gtk_tree_model_get(GTK_TREE_MODEL(model), &child, LST_DEFCAT_DATAS, &item, -1);
			if(item->key == key || item->parent == key)
			{
				DB( g_print(" + removing subcat %s\n", item->name) );
				gtk_tree_store_remove(GTK_TREE_STORE(model), &child);
			}

			cvalid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &child);
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
    }

	return TRUE;
}



void
ui_cat_listview_remove_selected(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("ui_cat_listview_remove_selected() \n") );

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
	}
}


static gboolean
ui_cat_listview_get_top_level (GtkTreeModel *liststore, guint32 key, GtkTreeIter *return_iter)
{
GtkTreeIter  iter;
gboolean     valid;
Category *item;

	DB( g_print("ui_cat_listview_get_top_level() \n") );

    if( liststore != NULL )
    {
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter);
		while (valid)
		{
			gtk_tree_model_get (liststore, &iter, LST_DEFCAT_DATAS, &item, -1);

			if(item->key == key)
			{
				*return_iter = iter;
				return TRUE;
			}

		 valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter);
		}
	}

	return FALSE;
}


static void ui_cat_listview_populate_cat_ghfunc(gpointer key, gpointer value, GtkTreeModel *model)
{
GtkTreeIter  toplevel;
Category *item = value;

	//DB( g_print("cat listview populate: %d %s\n", (guint32 *)key, item->name) );

	if( item->parent == 0 )
	{
		gtk_tree_store_append (GTK_TREE_STORE(model), &toplevel, NULL);

		gtk_tree_store_set (GTK_TREE_STORE(model), &toplevel,
			LST_DEFCAT_TOGGLE	, FALSE,
			LST_DEFCAT_DATAS, item,
			LST_DEFCAT_NAME, item->name,
			-1);
	}
}


static void ui_cat_listview_populate_subcat_ghfunc(gpointer key, gpointer value, GtkTreeModel *model)
{
GtkTreeIter  toplevel, child;
Category *item = value;
gboolean ret;


	if( item->parent != 0 )
	{
		ret = ui_cat_listview_get_top_level(model, item->parent, &toplevel);
		if( ret == TRUE )
		{
			gtk_tree_store_append (GTK_TREE_STORE(model), &child, &toplevel);

			gtk_tree_store_set (GTK_TREE_STORE(model), &child,
				LST_DEFCAT_TOGGLE	, FALSE,
				LST_DEFCAT_DATAS, item,
				LST_DEFCAT_NAME, item->name,
				-1);

		}
	}

}


static void ui_cat_listview_sort_force(GtkTreeSortable *sortable, gpointer user_data)
{
gint sort_column_id;
GtkSortType order;

	DB( g_print("ui_cat_listview_sort_force()\n") );

	gtk_tree_sortable_get_sort_column_id(sortable, &sort_column_id, &order);
	DB( g_print(" - id %d order %d\n", sort_column_id, order) );

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, order);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), sort_column_id, order);
}




void ui_cat_listview_populate(GtkWidget *view)
{
GtkTreeModel *model;

	DB( g_print("ui_cat_listview_populate() \n") );


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_tree_store_clear (GTK_TREE_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* we have to do this in 2 times to ensure toplevel (cat) will be added before childs */
	/* populate cat 1st */
	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)ui_cat_listview_populate_cat_ghfunc, model);
	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)ui_cat_listview_populate_subcat_ghfunc, model);


	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);

	gtk_tree_view_expand_all (GTK_TREE_VIEW(view));

}


GtkWidget *
ui_cat_listview_new(gboolean withtoggle)
{
GtkTreeStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	DB( g_print("ui_cat_listview_new() \n") );

	/* create tree store */
	store = gtk_tree_store_new(
		NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_STRING
		);

	//treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);


	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes ("Show", renderer, "active", LST_DEFCAT_TOGGLE, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (G_OBJECT(renderer), "toggled",
			    G_CALLBACK (ui_cat_listview_fixed_toggled), store);

	}

	// column 1
	column = gtk_tree_view_column_new();
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cat_listview_text_cell_data_function, GINT_TO_POINTER(LST_DEFCAT_NAME), NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DEFACC_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	// parameters
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, ui_cat_listview_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_DATAS, GTK_SORT_ASCENDING);

	//gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
	//gtk_tree_view_set_expander_column(GTK_TREE_VIEW(treeview), column);

	return treeview;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

//todo amiga/linux
//add exist function + check before add
//save
//load



/**
 * ui_cat_manage_filter_text_handler
 *
 *	filter to entry to avoid seizure of ':' char
 *
 */
static void ui_cat_manage_filter_text_handler (GtkEntry    *entry,
                          const gchar *text,
                          gint         length,
                          gint        *position,
                          gpointer     data)
{
GtkEditable *editable = GTK_EDITABLE(entry);
gint i, count=0, pos;
gchar *result = g_new0 (gchar, length+1);

  for (i=0; i < length; i++)
  {
    if (text[i]==':')
      continue;
    result[count++] = text[i];
  }


  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (ui_cat_manage_filter_text_handler),
                                     data);
    gtk_editable_insert_text (editable, result, count, &pos);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (ui_cat_manage_filter_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

  g_free (result);
}




/**
 * ui_cat_manage_dialog_load_csv:
 *
 */
static void
ui_cat_manage_dialog_load_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
gchar *filename = NULL;
gchar *error;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_cat_manage_dialog) load csv - data %p\n", data) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		if(!category_load_csv(filename, &error))
		{
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The csv file must contains the exact numbers of column,\nseparated by a semi-colon, please see the help for more details.")
					);
		}

		g_free( filename );
		ui_cat_listview_populate(data->LV_cat);
	}

}

/**
 * ui_cat_manage_dialog_save_csv:
 *
 */
static void
ui_cat_manage_dialog_save_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
gchar *filename = NULL;
gchar *error;

	DB( g_print("(defcategory) save csv\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_SAVE, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		category_save_csv(filename, &error);
		g_free( filename );
	}
}

/**
 * ui_cat_manage_dialog_add:
 *
 * add an empty new category/subcategory
 *
 */
static void
ui_cat_manage_dialog_add(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
gboolean subcat = GPOINTER_TO_INT(user_data);
const gchar *name;
//GtkTreeModel *model;
GtkTreeIter  parent_iter;
GtkWidget *tmpwidget;
Category *item, *paritem;
gboolean type;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) add (data=%p) is subcat=%d\n", data, subcat) );

	tmpwidget = (subcat == FALSE ? data->ST_name1 : data->ST_name2);
	name = gtk_entry_get_text(GTK_ENTRY(tmpwidget));
	//model  = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));

	/* ignore if item is empty */
	if (name && *name)
	{
		data->change++;

		item = da_cat_malloc();
		item->name = g_strdup(name);

		g_strstrip(item->name);

		/* if cat use new id */
		if(subcat == FALSE)
		{
			type = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_type));
			if(type == TRUE)
				item->flags |= GF_INCOME;

			if( da_cat_append(item) )
			{
				DB( g_print(" => add cat: %p %d, %s type=%d\n", item, subcat, item->name, type) );
				ui_cat_listview_add(GTK_TREE_VIEW(data->LV_cat), item, NULL);
			}
		}
		/* if subcat use parent id & gf_income */
		else
		{
			paritem = ui_cat_listview_get_selected_parent(GTK_TREE_VIEW(data->LV_cat), &parent_iter);
			if(paritem)
			{
				DB( g_print(" => selitem parent: %d, %s\n", paritem->key, paritem->name) );

				item->parent = paritem->key;
				item->flags |= (paritem->flags & GF_INCOME);
				item->flags |= GF_SUB;

				if(da_cat_append(item))
				{
					DB( g_print(" => add subcat: %p %d, %s\n", item, subcat, item->name) );
					ui_cat_listview_add(GTK_TREE_VIEW(data->LV_cat), item, &parent_iter);
				}
			}
		}

		gtk_entry_set_text(GTK_ENTRY(tmpwidget),"");
	}
}


static void ui_cat_manage_dialog_modify_entry_cb(GtkEditable *editable, gpointer user_data)
{
GtkDialog *window = user_data;
const gchar *buffer;

	buffer = gtk_entry_get_text(GTK_ENTRY(editable));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


/*
**
*/
static void
ui_cat_manage_dialog_modify(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
GtkWidget *window, *content, *mainvbox, *w_name, *w_type = NULL;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) modify\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

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
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_MAINBOX_SPACING);

		w_name = gtk_entry_new();
		gtk_box_pack_start (GTK_BOX (mainvbox), w_name, TRUE, TRUE, 0);

		gtk_entry_set_text(GTK_ENTRY(w_name), item->name);
		gtk_widget_grab_focus (w_name);

		gtk_entry_set_activates_default (GTK_ENTRY(w_name), TRUE);

		if(!(item->flags & GF_SUB))
		{
			w_type = gtk_check_button_new_with_mnemonic(_("_Income"));
			gtk_box_pack_start (GTK_BOX (mainvbox), w_type, TRUE, TRUE, 0);
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w_type), item->flags & GF_INCOME ? TRUE : FALSE);
		}

		gtk_widget_show_all(mainvbox);

		g_signal_connect (G_OBJECT (w_name), "changed", G_CALLBACK (ui_cat_manage_dialog_modify_entry_cb), window);
		
		gtk_dialog_set_default_response(GTK_DIALOG( window ), GTK_RESPONSE_ACCEPT);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			// 1: manage renaming
			name = gtk_entry_get_text(GTK_ENTRY(w_name));
			// ignore if item is empty
			if (name && *name)
			{
				if( category_rename(item, name) )
				{
					data->change++;
				}
				else
				{
				Category *parent;
				gchar *fromname, *toname = NULL;

					fromname = da_cat_get_fullname(item);

					if( item->parent == 0)
						toname = g_strdup(name);
					else
					{
						parent = da_cat_get(item->parent);
						if( parent )
						{
							toname = g_strdup_printf("%s:%s", parent->name, name);
						}
					}


					ui_dialog_msg_infoerror(GTK_WINDOW(window), GTK_MESSAGE_ERROR,
						_("Error"),
						_("Cannot rename this Category,\n"
						"from '%s' to '%s',\n"
						"this name already exists."),
						fromname,
						toname
						);

					g_free(fromname);
					g_free(toname);

				}
			}
	
			// 2: manage flag change
			if(!(item->flags & GF_SUB))
			{
			gboolean isIncome;
			
				isIncome = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w_type));
				data->change += category_change_type(item, isIncome);
			}
			
			ui_cat_listview_sort_force(GTK_TREE_SORTABLE(model), NULL);
	    }

		// cleanup and destroy
		gtk_widget_destroy (window);
	}

}


static void ui_cat_manage_dialog_move_entry_cb(GtkComboBox *widget, gpointer user_data)
{
GtkDialog *window = user_data;
gchar *buffer;

	buffer = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (widget))));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


/*
**
*/
static void ui_cat_manage_dialog_move(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
GtkWidget *window, *content, *mainvbox, *getwidget;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defcategory) move\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

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

		getwidget = ui_cat_comboboxentry_new(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);

		gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, FALSE);
		
		gtk_widget_show_all(mainvbox);

		g_signal_connect (G_OBJECT (getwidget), "changed", G_CALLBACK (ui_cat_manage_dialog_move_entry_cb), window);
		
		ui_cat_comboboxentry_populate_except(GTK_COMBO_BOX(getwidget), GLOBALS->h_cat, item->key);
		gtk_widget_grab_focus (getwidget);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (window));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		gboolean result;
		gchar *npn;

			npn = ui_cat_comboboxentry_get_name(GTK_COMBO_BOX(getwidget)),

			result = ui_dialog_msg_question(
				GTK_WINDOW(window),
				_("Move this category to another one ?"),
				_("This will replace '%s' by '%s',\n"
				  "and then remove '%s'"),
				item->name,
				npn,
				item->name,
				NULL
				);

			if( result == GTK_RESPONSE_YES )
			{
			Category *newcat, *parent;
			guint dstkey;

				dstkey = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(getwidget));
				newcat = da_cat_get (dstkey);

				DB( g_print(" moving to %d\n", dstkey) );

				category_move(item->key, dstkey);

				//keep the income with us
				parent = da_cat_get(item->parent);
				if(parent != NULL && (parent->flags & GF_INCOME))
					newcat->flags |= GF_INCOME;

				// remove the old category
				da_cat_remove(item->key);
				ui_cat_listview_remove_selected(GTK_TREE_VIEW(data->LV_cat));

				//add the new category into listview
				if(newcat)
					ui_cat_listview_add(GTK_TREE_VIEW(data->LV_cat), newcat, NULL);

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
static void ui_cat_manage_dialog_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
Category *item;
gint result;
gboolean do_remove;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) remove (data=%x)\n", (guint)data) );

	do_remove = TRUE;
	item = ui_cat_listview_get_selected(GTK_TREE_VIEW(data->LV_cat));
	if( item != NULL )
	{
		if( category_is_used(item->key) == TRUE )
		{
			result = ui_dialog_msg_question(
				GTK_WINDOW(data->window),
				_("Remove a category ?"),
				_("If you remove '%s', archive and transaction referencing this category\n"
				"will set place to 'no category'"),
				item->name,
				NULL
				);

			if( result == GTK_RESPONSE_YES )
			{
				category_move(item->key, 0);
			}
			else if( result == GTK_RESPONSE_NO )
			{
				do_remove = FALSE;
			}
		}

		if( do_remove )
		{
			ui_cat_listview_remove(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat)), item->key);
			da_cat_remove(item->key);
			data->change++;
		}
	}

}


/**
 * ui_cat_manage_dialog_update:
 *
 */
static void
ui_cat_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
//gint count;
gboolean selected, sensitive;
GtkTreeSelection *selection;
GtkTreeModel     *model;
GtkTreeIter       iter;
GtkTreePath *path;
gchar *category;
gboolean haschild = FALSE;

	DB( g_print("ui_cat_manage_dialog_update()\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_print("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &model, &iter);

	DB( g_print(" selected = %d\n", selected) );

	if(selected)
	{
		//path 0 active ?
		gtk_tree_model_get_iter_first(model, &iter);
		if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat)), &iter))
		{
			DB( g_print(" 0 active = %d\n", 1) );
			selected = FALSE;
		}
	}

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	//count = gtk_tree_selection_count_selected_rows(selection);
	//DB( g_print(" => select count=%d\n", count) );

	category = NULL;
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	gchar *tree_path_str;
	Category *item;

		gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
			LST_DEFCAT_DATAS, &item,
			-1);

		haschild = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(model), &iter);
		DB( g_print(" => has child=%d\n", haschild) );


		path = gtk_tree_model_get_path(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)), &iter);
		tree_path_str = gtk_tree_path_to_string(path);
		DB( g_print(" => select is=%s, depth=%d (id=%d, %s) flags=%d\n",
			tree_path_str,
			gtk_tree_path_get_depth(path),
			item->key,
			item->name,
			item->flags
			) );
		g_free(tree_path_str);

		//get parent if subcategory selectd
		DB( g_print(" => get parent for title\n") );
		if(gtk_tree_path_get_depth(path) != 1)
			gtk_tree_path_up(path);

		if(gtk_tree_model_get_iter(model, &iter, path))
		{
		Category *tmpitem;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFCAT_DATAS, &tmpitem,
				-1);

			category = tmpitem->name;


			DB( g_print(" => parent is %s\n", category) );

		}

		gtk_tree_path_free(path);

	}

	gtk_label_set_text(GTK_LABEL(data->LA_category), category);

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name2, sensitive);
	gtk_widget_set_sensitive(data->BT_add2, sensitive);
	gtk_widget_set_sensitive(data->BT_mov, sensitive);
	gtk_widget_set_sensitive(data->BT_mod, sensitive);

	//avoid removing top categories
	sensitive = (haschild == TRUE) ? FALSE : sensitive;

	gtk_widget_set_sensitive(data->BT_rem, sensitive);
}


/*
**
*/
static void ui_cat_manage_dialog_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_cat_manage_dialog_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

static void ui_cat_manage_dialog_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            user_data)
{
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("ui_cat_manage_dialog_onRowActivated()\n") );


	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter_first(model, &iter);
	if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	{
		ui_cat_manage_dialog_modify(GTK_WIDGET(treeview), NULL);
	}
}




/*
**
*/
static gboolean ui_cat_manage_dialog_cleanup(struct ui_cat_manage_dialog_data *data, gint result)
{
gboolean doupdate = FALSE;

	DB( g_print("(defcategory) cleanup\n") );

	if(result == GTK_RESPONSE_ACCEPT)
	{

		//do_application_specific_something ();
		DB( g_print(" accept\n") );


		GLOBALS->changes_count += data->change;
	}

	DB( g_print(" free tmp_list\n") );

	//da_category_destroy(data->tmp_list);

	return doupdate;
}

/*
**
*/
static void ui_cat_manage_dialog_setup(struct ui_cat_manage_dialog_data *data)
{
	DB( g_print("(defcategory) setup\n") );

	//init GList
	data->tmp_list = NULL; //data->tmp_list = hb-glist_clone_list(GLOBALS->cat_list, sizeof(struct _Group));
	data->change = 0;

	//debug
	//da_cat_debug_list();


	ui_cat_listview_populate(data->LV_cat);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));

}

// the window creation
GtkWidget *ui_cat_manage_dialog (void)
{
struct ui_cat_manage_dialog_data data;
GtkWidget *window, *content, *mainvbox, *table, *hbox, *label, *scrollwin, *vbox, *separator, *treeview;
gint row;

	window = gtk_dialog_new_with_buttons (_("Manage Categories"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    GTK_STOCK_CLOSE,
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;

	//set the window icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (window), "category.svg");
	gtk_window_set_icon_name(GTK_WINDOW (window), HB_STOCK_CATEGORY);

	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(defcategory) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	//window contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainvbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), HB_MAINBOX_SPACING);

    //our table
	table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	// category item + add button
	row = 0;
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.ST_name1 = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), data.ST_name1, TRUE, TRUE, 0);
	data.CM_type = gtk_check_button_new_with_mnemonic(_("I_ncome"));
	gtk_box_pack_start (GTK_BOX (hbox), data.CM_type, FALSE, FALSE, 0);

	data.BT_add1 = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add1, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	// subcategory + add button
	row++;
	hbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	data.LA_category = gtk_label_new(NULL);
	gtk_box_pack_start (GTK_BOX (hbox), data.LA_category, FALSE, FALSE, 0);
	label = gtk_label_new(":");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data.ST_name2 = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (hbox), data.ST_name2, TRUE, TRUE, 0);
	data.BT_add2 = gtk_button_new_from_stock(GTK_STOCK_ADD);
	gtk_table_attach (GTK_TABLE (table), data.BT_add2, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);




	//list
	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_table_attach (GTK_TABLE (table), scrollwin, 0, 1, row, row+1, (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	//gtk_container_set_border_width (GTK_CONTAINER(scrollwin), 5);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 	treeview = (GtkWidget *)ui_cat_listview_new(FALSE);
	gtk_widget_set_size_request(treeview, HB_MINWIDTH_LIST, -1);
 	data.LV_cat = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);

	vbox = gtk_vbox_new (FALSE, HB_BOX_SPACING);
	gtk_table_attach (GTK_TABLE (table), vbox, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND|GTK_FILL), 0, 0);

	/*
	widget = gtk_check_button_new_with_mnemonic("Income type");
	data.CM_type = widget;
	gtk_box_pack_start (GTK_BOX (vbox), data.CM_type, FALSE, FALSE, 0);
	*/

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
	g_signal_connect (G_OBJECT (data.ST_name1), "activate", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(FALSE));
	g_signal_connect (G_OBJECT (data.ST_name2), "activate", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(TRUE));

	g_signal_connect(G_OBJECT(data.ST_name1), "insert_text", G_CALLBACK(ui_cat_manage_filter_text_handler), NULL);
	g_signal_connect(G_OBJECT(data.ST_name2), "insert_text", G_CALLBACK(ui_cat_manage_filter_text_handler), NULL);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cat)), "changed", G_CALLBACK (ui_cat_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_cat), "row-activated", G_CALLBACK (ui_cat_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_add1), "clicked", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(FALSE));
	g_signal_connect (G_OBJECT (data.BT_add2), "clicked", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(TRUE));
	g_signal_connect (G_OBJECT (data.BT_mod), "clicked", G_CALLBACK (ui_cat_manage_dialog_modify), NULL);
	g_signal_connect (G_OBJECT (data.BT_mov), "clicked", G_CALLBACK (ui_cat_manage_dialog_move), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_cat_manage_dialog_remove), NULL);

	g_signal_connect (G_OBJECT (data.BT_import), "clicked", G_CALLBACK (ui_cat_manage_dialog_load_csv), NULL);
	g_signal_connect (G_OBJECT (data.BT_export), "clicked", G_CALLBACK (ui_cat_manage_dialog_save_csv), NULL);

	//setup, init and show window
	ui_cat_manage_dialog_setup(&data);
	ui_cat_manage_dialog_update(data.LV_cat, NULL);

	gtk_window_resize(GTK_WINDOW(window), 200, 320);


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
	ui_cat_manage_dialog_cleanup(&data, result);
	gtk_widget_destroy (window);

  return NULL;
}



