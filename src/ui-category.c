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


gchar *CYA_CAT_TYPE[] = { 
	N_("Expense"), 
	N_("Income"), 
	NULL
};

static void ui_cat_manage_populate_listview(struct ui_cat_manage_dialog_data *data);

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


Category
*ui_cat_comboboxentry_get(GtkComboBox *entry_box)
{
Category *item = NULL;
gchar *name;

	DB( g_print ("ui_cat_comboboxentry_get_key()\n") );

	name = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	if(name == NULL)
		return NULL;

	item = da_cat_get_by_fullname(name);

	return item;
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
			LST_CMBCAT_DATAS, item,
			LST_CMBCAT_FULLNAME, fullname,
			LST_CMBCAT_SORTNAME, NULL,
			LST_CMBCAT_NAME, name,
			LST_CMBCAT_SUBCAT, item->parent == 0 ? 1 : 0,
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
Category *pitem = NULL;
gchar *fullname, *name, *sortname;
gchar type;

	if( ( item->key != ctx->except_key ) )
	{
		pitem = da_cat_get(item->parent);

		type = (item->flags & GF_INCOME) ? '+' : '-';
		fullname = da_cat_get_fullname(item);
		sortname = NULL;
		name = NULL;
		
		//DB( g_print ("cat combo populate [%d:%d] %s\n", item->parent, item->key, fullname) );

		if(item->key == 0)
		{
			name = g_strdup(item->name);
			sortname = g_strdup(item->name);
		}
		else
		{
			if( item->parent == 0 )
			{
				name = g_strdup_printf("%s [%c]", item->name, type);
				sortname = g_strdup_printf("%s", item->name);
			}
			else
			{
				if(pitem)
				{
					name = g_strdup_printf(" %c %s", type, item->name);
					sortname = g_strdup_printf("%s_%s", pitem->name, item->name);
				}
			}
		}
		
		hb_string_replace_char(' ', sortname);
		
		//gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		//gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter,
		gtk_list_store_insert_with_values(GTK_LIST_STORE(ctx->model), &iter, -1,
			LST_CMBCAT_DATAS, item,
			LST_CMBCAT_FULLNAME, fullname,
			LST_CMBCAT_SORTNAME, sortname,
			LST_CMBCAT_NAME, name,
		    LST_CMBCAT_SUBCAT, item->parent == 0 ? 1 : 0,
			-1);

		DB( g_print(" - add [%2d:%2d] '%-12s' '%-12s' '%s' '%s' %d\n", item->parent, item->key, pitem->name, name, fullname, sortname, item->parent == 0 ? 1 : 0) );

		g_free(sortname);
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
//GtkEntryCompletion *completion;
struct catPopContext ctx;

    DB( g_print ("ui_cat_comboboxentry_populate()\n") );

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
	
	g_hash_table_foreach(hash, (GHFunc)ui_cat_comboboxentry_populate_ghfunc, &ctx);

	/* reatach our model */
	//gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	//gtk_entry_completion_set_model (completion, model);
	//g_object_unref(model);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

}



static gint
ui_cat_comboboxentry_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint retval = 0;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, 
		LST_CMBCAT_SORTNAME, &name1,
		 -1);
    gtk_tree_model_get(model, b, 
		LST_CMBCAT_SORTNAME, &name2,
		 -1);

	//DB( g_print(" compare '%s' '%s'\n", name1, name2) );

	retval = hb_string_utf8_compare(name1, name2);

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
		LST_CMBCAT_NAME, &name,
	    LST_CMBCAT_SUBCAT, &subcat,
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
                      LST_CMBCAT_FULLNAME, &item,
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

	store = gtk_list_store_new (NUM_LST_CMBCAT,
		G_TYPE_POINTER,
		G_TYPE_STRING, 			//fullname Car:Fuel
		G_TYPE_STRING,			//parent name Car
		G_TYPE_STRING, 			//name	Car or Fuel
	    G_TYPE_BOOLEAN			//subcat = 1
		);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_cat_comboboxentry_compare_func, NULL, NULL);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
	//g_object_set(completion, "text-column", LST_CMBCAT_FULLNAME, NULL);
	gtk_entry_completion_set_match_func(completion, ui_cat_comboboxentry_completion_func, NULL, NULL);
	//gtk_entry_completion_set_minimum_key_length(completion, 2);

	gtk_entry_completion_set_text_column(completion, 1);

	/*renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (completion), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (completion), renderer, "text", LST_CMBCAT_FULLNAME, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (completion),
					    renderer,
					    ui_cat_comboboxentry_test,
					    NULL, NULL);
	*/

	// dothe same for combobox

	comboboxentry = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(comboboxentry), LST_CMBCAT_FULLNAME);

	gtk_cell_layout_clear(GTK_CELL_LAYOUT (comboboxentry));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (comboboxentry), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (comboboxentry), renderer, "text", LST_CMBCAT_FULLNAME, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (comboboxentry),
					    renderer,
					    ui_cat_comboboxentry_test,
					    NULL, NULL);

	gtk_entry_set_completion (GTK_ENTRY (gtk_bin_get_child(GTK_BIN (comboboxentry))), completion);

	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), comboboxentry);

	gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_LIST, -1);

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

static gint
ui_cat_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint sortcol = GPOINTER_TO_INT(userdata);
Category *entry1, *entry2;
gint retval = 0;
	
	gtk_tree_model_get(model, a, LST_DEFCAT_DATAS, &entry1, -1);
	gtk_tree_model_get(model, b, LST_DEFCAT_DATAS, &entry2, -1);

    switch (sortcol)
    {
		case LST_DEFCAT_SORT_NAME:
			retval = (entry1->flags & GF_INCOME) - (entry2->flags & GF_INCOME);
			if(!retval)
			{
				retval = hb_string_utf8_compare(entry1->name, entry2->name);
			}
			break;
		case LST_DEFCAT_SORT_USED:
			retval = entry1->usage_count - entry2->usage_count;
			break;
		default:
			g_return_val_if_reached(0);
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


static void
ui_cat_listview_count_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Category *entry;
gchar buffer[256];

	gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &entry, -1);
	if(entry->usage_count > 0)
	{
		g_snprintf(buffer, 256-1, "%d", entry->usage_count);
		g_object_set(renderer, "text", buffer, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);
}


/* = = = = = = = = = = = = = = = = */


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


static void ui_cat_listview_populate_cat_ghfunc(gpointer key, gpointer value, struct catPopContext *ctx)
{
GtkTreeIter  toplevel;
Category *item = value;
gint item_type;

	item_type = (item->flags & GF_INCOME) ? CAT_TYPE_INCOME : CAT_TYPE_EXPENSE; 

	//DB( g_print("cat listview populate: %d %s\n", (guint32 *)key, item->name) );
	if( (ctx->type == CAT_TYPE_ALL) || ctx->type == item_type || item->key == 0 )
	{
		if( item->parent == 0 )
		{
			gtk_tree_store_append (GTK_TREE_STORE(ctx->model), &toplevel, NULL);

			gtk_tree_store_set (GTK_TREE_STORE(ctx->model), &toplevel,
				LST_DEFCAT_TOGGLE	, FALSE,
				LST_DEFCAT_DATAS, item,
				LST_DEFCAT_NAME, item->name,
				-1);
		}
	}
	
}


static void ui_cat_listview_populate_subcat_ghfunc(gpointer key, gpointer value, struct catPopContext *ctx)
{
GtkTreeIter  toplevel, child;
Category *item = value;
gboolean ret;
gint item_type;

	item_type = (item->flags & GF_INCOME) ? CAT_TYPE_INCOME : CAT_TYPE_EXPENSE; 

	if( (ctx->type == CAT_TYPE_ALL) || ctx->type == item_type)
	{
		if( item->parent != 0 )
		{
			ret = ui_cat_listview_get_top_level(ctx->model, item->parent, &toplevel);
			if( ret == TRUE )
			{
				gtk_tree_store_append (GTK_TREE_STORE(ctx->model), &child, &toplevel);

				gtk_tree_store_set (GTK_TREE_STORE(ctx->model), &child,
					LST_DEFCAT_TOGGLE	, FALSE,
					LST_DEFCAT_DATAS, item,
					LST_DEFCAT_NAME, item->name,
					-1);
			}
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


void ui_cat_listview_populate(GtkWidget *view, gint type)
{
GtkTreeModel *model;
struct catPopContext ctx = { 0 };

	DB( g_print("ui_cat_listview_populate() \n") );


	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_tree_store_clear (GTK_TREE_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* clear and populate */
	ctx.model = model;
	ctx.type  = type;
	
	/* we have to do this in 2 times to ensure toplevel (cat) will be added before childs */
	/* populate cat 1st */
	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)ui_cat_listview_populate_cat_ghfunc, &ctx);
	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)ui_cat_listview_populate_subcat_ghfunc, &ctx);


	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);

	gtk_tree_view_expand_all (GTK_TREE_VIEW(view));

}


static gboolean ui_cat_listview_search_equal_func (GtkTreeModel *model,
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
  Category *item;
	
  //gtk_tree_model_get_value (model, iter, column, &value);
  gtk_tree_model_get(model, iter, LST_DEFCAT_DATAS, &item, -1);

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
ui_cat_listview_new(gboolean withtoggle, gboolean withcount)
{
GtkTreeStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	DB( g_print("ui_cat_listview_new() \n") );

	store = gtk_tree_store_new(
		NUM_LST_DEFCAT,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER,
		G_TYPE_STRING
		);

	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);


	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
						renderer, "active", LST_DEFCAT_TOGGLE, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (G_OBJECT(renderer), "toggled",
			    G_CALLBACK (ui_cat_listview_fixed_toggled), store);

	}

	// column 2: name
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Name"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cat_listview_text_cell_data_function, GINT_TO_POINTER(LST_DEFCAT_NAME), NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST);
	gtk_tree_view_column_set_sort_column_id (column, LST_DEFCAT_SORT_NAME);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	if( withcount == TRUE )
	{
		column = gtk_tree_view_column_new();
		gtk_tree_view_column_set_title(column, _("Usage"));
		renderer = gtk_cell_renderer_text_new ();
		g_object_set(renderer, "xalign", 0.5, NULL);
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cat_listview_count_cell_data_function, GINT_TO_POINTER(LST_DEFCAT_DATAS), NULL);
		gtk_tree_view_column_set_alignment (column, 0.5);
		gtk_tree_view_column_set_sort_column_id (column, LST_DEFCAT_SORT_USED);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	}

	
	gtk_tree_view_set_search_equal_func(GTK_TREE_VIEW(treeview), ui_cat_listview_search_equal_func, NULL, NULL);

	// treeview attribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), withcount);

	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_SORT_NAME, ui_cat_listview_compare_func, GINT_TO_POINTER(LST_DEFCAT_SORT_NAME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCAT_SORT_USED, ui_cat_listview_compare_func, GINT_TO_POINTER(LST_DEFCAT_SORT_USED), NULL);
	
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCAT_SORT_NAME, GTK_SORT_ASCENDING);

	return treeview;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

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
gint i, count=0;
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
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (ui_cat_manage_filter_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

  g_free (result);
}


static void
ui_cat_manage_dialog_delete_unused( GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data = user_data;
gboolean result;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_cat_manage_dialog) delete unused - data %p\n", data) );

	result = ui_dialog_msg_confirm_alert(
			GTK_WINDOW(data->window),
			_("Delete unused categories"),
			_("Are you sure you want to permanently\ndelete unused categories?"),
			_("_Delete")
		);

	if( result == GTK_RESPONSE_OK )
	{
	GtkTreeModel *model;	
		
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
		gtk_tree_store_clear (GTK_TREE_STORE(model));

		category_delete_unused();
	
		ui_cat_manage_populate_listview (data);
	}
}



/**
 * ui_cat_manage_dialog_load_csv:
 *
 */
static void
ui_cat_manage_dialog_load_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data = user_data;
gchar *filename = NULL;
gchar *error;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	DB( g_print("(ui_cat_manage_dialog) load csv - data %p\n", data) );

	if( ui_file_chooser_csv(GTK_WINDOW(data->window), GTK_FILE_CHOOSER_ACTION_OPEN, &filename, NULL) == TRUE )
	{
		DB( g_print(" + filename is %s\n", filename) );

		if(!category_load_csv(filename, &error))
		{
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("File format error"),
					_("The CSV file must contains the exact numbers of column,\nseparated by a semi-colon, please see the help for more details.")
					);
		}

		g_free( filename );
		ui_cat_manage_populate_listview(data);
	}

}

/**
 * ui_cat_manage_dialog_save_csv:
 *
 */
static void
ui_cat_manage_dialog_save_csv( GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data = user_data;
gchar *filename = NULL;
gchar *error;

	DB( g_print("(defcategory) save csv\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

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
gint type;

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
			type = radio_get_active(GTK_CONTAINER(data->RA_type));
			if(type == 1)
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


static void ui_cat_manage_dialog_edit_entry_cb(GtkEditable *editable, gpointer user_data)
{
GtkDialog *window = user_data;
const gchar *buffer;

	buffer = gtk_entry_get_text(GTK_ENTRY(editable));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_ACCEPT, strlen(buffer) > 0 ? TRUE : FALSE);
}


static void ui_cat_manage_dialog_edit(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
GtkWidget *dialog, *content, *mainvbox, *w_name, *w_type = NULL;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) edit\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *item;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &item, -1);

		dialog = gtk_dialog_new_with_buttons (_("Edit..."),
						    GTK_WINDOW (data->window),
						    0,
						    _("_Cancel"),
						    GTK_RESPONSE_REJECT,
						    _("_OK"),
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
		mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), SPACING_MEDIUM);

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

		g_signal_connect (G_OBJECT (w_name), "changed", G_CALLBACK (ui_cat_manage_dialog_edit_entry_cb), dialog);


		gtk_widget_show_all(mainvbox);

		gtk_dialog_set_default_response(GTK_DIALOG( dialog ), GTK_RESPONSE_ACCEPT);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

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
					//to redraw the active entry
					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_cat));
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


					ui_dialog_msg_infoerror(GTK_WINDOW(dialog), GTK_MESSAGE_ERROR,
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
		gtk_widget_destroy (dialog);
	}

}


static void ui_cat_manage_dialog_merge_entry_cb(GtkComboBox *widget, gpointer user_data)
{
GtkDialog *window = user_data;
gchar *buffer;

	buffer = (gchar *)gtk_entry_get_text(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (widget))));
	gtk_dialog_set_response_sensitive(GTK_DIALOG(window), GTK_RESPONSE_OK, strlen(buffer) > 0 ? TRUE : FALSE);
}


static void ui_cat_manage_dialog_merge(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
GtkWidget *dialog, *content, *mainvbox;
GtkWidget *getwidget, *togglebutton;
GtkTreeSelection *selection;
GtkTreeModel *model;
GtkTreeIter iter;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("(defcategory) merge\n") );

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cat));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Category *srccat;
	gchar *title;
	gchar *secondtext;

		gtk_tree_model_get(model, &iter, LST_DEFCAT_DATAS, &srccat, -1);

		title = g_strdup_printf (
			_("Merge category '%s'"), srccat->name);

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

		secondtext = _("Transactions assigned to this category,\n"
			  "will be moved to the category selected below.");

		g_object_set(GTK_MESSAGE_DIALOG (dialog), "secondary-text", secondtext, NULL);
		g_free(title);

		getwidget = ui_cat_comboboxentry_new(NULL);
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, FALSE, FALSE, 0);

		secondtext = g_strdup_printf (
			_("_Delete the category '%s'"), srccat->name);
		togglebutton = gtk_check_button_new_with_mnemonic(secondtext);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(togglebutton), TRUE);
		g_free(secondtext);
		gtk_box_pack_start (GTK_BOX (mainvbox), togglebutton, FALSE, FALSE, 0);

		//setup 
		//gtk_combo_box_set_active(GTK_COMBO_BOX(getwidget), oldpos);
		g_signal_connect (G_OBJECT (getwidget), "changed", G_CALLBACK (ui_cat_manage_dialog_merge_entry_cb), dialog);
		gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK, FALSE);

		ui_cat_comboboxentry_populate_except(GTK_COMBO_BOX(getwidget), GLOBALS->h_cat, srccat->key);
		gtk_widget_grab_focus (getwidget);

		gtk_widget_show_all(mainvbox);
		
		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_OK)
		{
		GtkTreeModel *model;
		Category *newcat, *parent;
		guint dstcatkey;


			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat));
			gtk_tree_store_clear (GTK_TREE_STORE(model));

			dstcatkey = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(getwidget));

			DB( g_print(" -> move cat to %d\n", dstcatkey) );

			category_move(srccat->key, dstcatkey);

			newcat = da_cat_get (dstcatkey);

			//keep the income type with us
			parent = da_cat_get(srccat->parent);
			if(parent != NULL && (parent->flags & GF_INCOME))
				newcat->flags |= GF_INCOME;

			//add the new category into listview
			if(newcat)
				ui_cat_listview_add(GTK_TREE_VIEW(data->LV_cat), newcat, NULL);

			// delete the old category
			if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(togglebutton)) )
			{
				DB( g_print(" -> delete %d '%s'\n", srccat->key, srccat->name ) );

				da_cat_remove(srccat->key);
				ui_cat_listview_remove_selected(GTK_TREE_VIEW(data->LV_cat));
			}


			data->change++;

			ui_cat_manage_populate_listview(data);

		}

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	}

}


/*
** delete the selected payee to our treeview and temp GList
*/
static void ui_cat_manage_dialog_delete(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;
Category *item;
gint result;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) delete (data=%x)\n", (guint)data) );

	item = ui_cat_listview_get_selected(GTK_TREE_VIEW(data->LV_cat));
	if( item != NULL )
	{
	gchar *title = NULL;
	gchar *secondtext = NULL;

		title = g_strdup_printf (
			_("Are you sure you want to permanently delete '%s'?"), item->name);

		if( item->usage_count > 0 )
		{
			secondtext = _("This category is used.\n"
			    "Any transaction using that category will be set to (no category)");
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
			category_move(item->key, 0);
			ui_cat_listview_remove(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cat)), item->key);
			da_cat_remove(item->key);
			data->change++;
		}

	}
}



static void ui_cat_manage_dialog_expand_all(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) expand all (data=%x)\n", (guint)data) );

	gtk_tree_view_expand_all(GTK_TREE_VIEW(data->LV_cat));

}


static void ui_cat_manage_dialog_collapse_all(GtkWidget *widget, gpointer user_data)
{
struct ui_cat_manage_dialog_data *data;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(defcategory) collapse all (data=%x)\n", (guint)data) );

	gtk_tree_view_collapse_all(GTK_TREE_VIEW(data->LV_cat));

}


static void ui_cat_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
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

			if(tmpitem->key > 0)
				category = tmpitem->name;

			DB( g_print(" => parent is %s\n", category) );

		}

		gtk_tree_path_free(path);

	}

	gtk_label_set_text(GTK_LABEL(data->LA_category), category);

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->ST_name2, sensitive);
	gtk_widget_set_sensitive(data->BT_edit, sensitive);
	gtk_widget_set_sensitive(data->BT_merge, sensitive);

	//avoid removing top categories
	sensitive = (haschild == TRUE) ? FALSE : sensitive;

	gtk_widget_set_sensitive(data->BT_delete, sensitive);
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
		ui_cat_manage_dialog_edit(GTK_WIDGET(treeview), NULL);
	}
}


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


static void ui_cat_manage_populate_listview(struct ui_cat_manage_dialog_data *data)
{
gint type;

	type = radio_get_active(GTK_CONTAINER(data->RA_type)) == 1 ? CAT_TYPE_INCOME : CAT_TYPE_EXPENSE;
	ui_cat_listview_populate(data->LV_cat, type);
	gtk_tree_view_expand_all (GTK_TREE_VIEW(data->LV_cat));
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

	ui_cat_manage_populate_listview(data);
	
}


static void ui_cat_manage_type_changed_cb (GtkToggleButton *button, gpointer user_data)
{
	ui_cat_manage_populate_listview(user_data);
	//g_print(" toggle type=%d\n", gtk_toggle_button_get_active(button));
}


GtkWidget *ui_cat_manage_dialog (void)
{
struct ui_cat_manage_dialog_data data;
GtkWidget *window, *content, *mainvbox, *bbox, *table, *hbox, *vbox, *label, *scrollwin, *treeview;
GtkWidget *menu, *menuitem, *widget, *image, *tbar;
GtkToolItem *toolitem;
gint w, h, row;

	window = gtk_dialog_new_with_buttons (_("Manage Categories"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    _("_Close"),
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = window;
	data.change = 0;

	gtk_window_set_icon_name(GTK_WINDOW (window), ICONNAME_HB_CATEGORY);

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(window), -1, h/PHI);

	
	//store our window private data
	g_object_set_data(G_OBJECT(window), "inst_data", (gpointer)&data);
	DB( g_print("(defcategory) window=%x, inst_data=%x\n", (guint)window, (guint)&data) );

    g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);

	//window contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (window));
	mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_SMALL);
	gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainvbox), SPACING_MEDIUM);

    //our table
	table = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (mainvbox), table, TRUE, TRUE, 0);

	row = 0;
	bbox = make_radio(CYA_CAT_TYPE, TRUE, GTK_ORIENTATION_HORIZONTAL);
	data.RA_type = bbox;
	gtk_widget_set_halign (bbox, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (table), bbox, 0, row, 2, 1);

	widget = radio_get_nth_widget(GTK_CONTAINER(bbox), 1);
	if(widget)
		g_signal_connect (widget, "toggled", G_CALLBACK (ui_cat_manage_type_changed_cb), &data);

	menu = gtk_menu_new ();
	gtk_widget_set_halign (menu, GTK_ALIGN_END);

	menuitem = gtk_menu_item_new_with_mnemonic (_("_Import CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_cat_manage_dialog_load_csv), &data);

	menuitem = gtk_menu_item_new_with_mnemonic (_("E_xport CSV"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_cat_manage_dialog_save_csv), &data);

	menuitem = gtk_separator_menu_item_new();
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	
	menuitem = gtk_menu_item_new_with_mnemonic (_("_Delete unused"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
	g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (ui_cat_manage_dialog_delete_unused), &data);

	gtk_widget_show_all (menu);
	
	widget = gtk_menu_button_new();
	image = gtk_image_new_from_icon_name (ICONNAME_HB_BUTTON_MENU, GTK_ICON_SIZE_MENU);

	//gchar *thename;
	//gtk_image_get_icon_name(image, &thename, NULL);
	//g_print("the name is %s\n", thename);

	g_object_set (widget, "image", image, "popup", GTK_MENU(menu),  NULL);
	gtk_widget_set_halign (widget, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);

	row++;
	widget = gtk_entry_new ();
	data.ST_name1 = widget;
	gtk_entry_set_placeholder_text(GTK_ENTRY(data.ST_name1), _("new category") );
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (table), widget, 0, row, 2, 1);

	// subcategory + add button
	row++;
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (table), hbox, 0, row, 2, 1);
	data.LA_category = gtk_label_new(NULL);
	gtk_box_pack_start (GTK_BOX (hbox), data.LA_category, FALSE, FALSE, 0);
	label = gtk_label_new(":");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	data.ST_name2 = gtk_entry_new ();
	gtk_entry_set_placeholder_text(GTK_ENTRY(data.ST_name2), _("new subcategory") );
	gtk_box_pack_start (GTK_BOX (hbox), data.ST_name2, TRUE, TRUE, 0);

	//list
	row++;
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_grid_attach (GTK_GRID (table), vbox, 0, row, 2, 1);
	
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwin), HB_MINHEIGHT_LIST);
 	treeview = ui_cat_listview_new(FALSE, TRUE);
	data.LV_cat = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_widget_set_hexpand (scrollwin, TRUE);
	gtk_widget_set_vexpand (scrollwin, TRUE);
	gtk_box_pack_start (GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

	//list toolbar
	tbar = gtk_toolbar_new();
	gtk_toolbar_set_icon_size (GTK_TOOLBAR(tbar), GTK_ICON_SIZE_MENU);
	gtk_toolbar_set_style(GTK_TOOLBAR(tbar), GTK_TOOLBAR_ICONS);
	gtk_style_context_add_class (gtk_widget_get_style_context (tbar), GTK_STYLE_CLASS_INLINE_TOOLBAR);
	gtk_box_pack_start (GTK_BOX (vbox), tbar, FALSE, FALSE, 0);

	/*widget = gtk_tool_item_new ();
	label = gtk_label_new("test");
	gtk_container_add(GTK_CONTAINER(widget), label);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(widget), -1);*/
	
	//hbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	/*
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);
	
		//widget = make_image_button("text-editor-symbolic", _("Edit"));
		widget = gtk_button_new_with_mnemonic(_("_Edit"));
		data.BT_edit = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

		//widget = make_image_button("merge-symbolic", _("Merge"));
		widget = gtk_button_new_with_mnemonic(_("_Merge"));
		data.BT_merge = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	
		//widget = make_image_button(ICONNAME_SYM_EDIT_DELETE, _("Delete"));
		widget = gtk_button_new_with_mnemonic(_("_Delete"));
		data.BT_delete = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
	*/
	
	toolitem = gtk_separator_tool_item_new ();
	gtk_tool_item_set_expand (toolitem, TRUE);
	gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolitem), FALSE);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	toolitem = gtk_tool_item_new();
	gtk_container_add (GTK_CONTAINER(toolitem), hbox);
	gtk_toolbar_insert(GTK_TOOLBAR(tbar), GTK_TOOL_ITEM(toolitem), -1);
	
		widget = make_image_button(ICONNAME_HB_BUTTON_EXPAND, _("Expand all"));
		data.BT_expand = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_HB_BUTTON_COLLAPSE, _("Collapse all"));
		data.BT_collapse = widget;
		gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);


	row++;
	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (table), bbox, 0, row, 2, 1);

	data.BT_edit = gtk_button_new_with_mnemonic(_("_Edit"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_edit);

	data.BT_merge = gtk_button_new_with_mnemonic(_("_Merge"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_merge);

	data.BT_delete = gtk_button_new_with_mnemonic(_("_Delete"));
	gtk_container_add (GTK_CONTAINER (bbox), data.BT_delete);


	/*row++;
	widget = gtk_check_button_new_with_mnemonic(_("I_ncome"));
	data.CM_type = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 0, row, 3, 1);*/


	//connect all our signals
	g_signal_connect (G_OBJECT (data.ST_name1), "activate", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(FALSE));
	g_signal_connect (G_OBJECT (data.ST_name2), "activate", G_CALLBACK (ui_cat_manage_dialog_add), GINT_TO_POINTER(TRUE));

	g_signal_connect(G_OBJECT(data.ST_name1), "insert-text", G_CALLBACK(ui_cat_manage_filter_text_handler), NULL);
	g_signal_connect(G_OBJECT(data.ST_name2), "insert-text", G_CALLBACK(ui_cat_manage_filter_text_handler), NULL);


	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cat)), "changed", G_CALLBACK (ui_cat_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_cat), "row-activated", G_CALLBACK (ui_cat_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_edit), "clicked", G_CALLBACK (ui_cat_manage_dialog_edit), NULL);
	g_signal_connect (G_OBJECT (data.BT_merge), "clicked", G_CALLBACK (ui_cat_manage_dialog_merge), NULL);
	g_signal_connect (G_OBJECT (data.BT_delete), "clicked", G_CALLBACK (ui_cat_manage_dialog_delete), NULL);

	g_signal_connect (G_OBJECT (data.BT_expand), "clicked", G_CALLBACK (ui_cat_manage_dialog_expand_all), NULL);
	g_signal_connect (G_OBJECT (data.BT_collapse), "clicked", G_CALLBACK (ui_cat_manage_dialog_collapse_all), NULL);

	//setup, init and show window
	category_fill_usage();
	ui_cat_manage_dialog_setup(&data);
	ui_cat_manage_dialog_update(data.LV_cat, NULL);

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

	GLOBALS->changes_count += data.change;

	return NULL;
}



