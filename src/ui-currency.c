/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2018 Maxime DOYEN
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

#include "ui-currency.h"


#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;
extern struct Preferences *PREFS;

extern Currency4217 iso4217cur[];
extern guint n_iso4217cur;


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ui_cur_combobox_get_name:
 *
 * get the name of the active curee or -1
 *
 * Return value: a new allocated name tobe freed with g_free
 *
 */
gchar *
ui_cur_combobox_get_name(GtkComboBox *entry_box)
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
 * ui_cur_combobox_get_key:
 *
 * get the key of the active curee
 *
 * Return value: the key or 0
 *
 */
guint32
ui_cur_combobox_get_key(GtkComboBox *entry_box)
{
GtkTreeModel *model;
GtkTreeIter	iter;
guint32 key = 0;

	if (gtk_combo_box_get_active_iter(entry_box, &iter) == TRUE)
	{
		model = gtk_combo_box_get_model(entry_box);
		gtk_tree_model_get (model, &iter, 1, &key, -1);
	}

	return key;
}


gboolean
ui_cur_combobox_set_active(GtkComboBox *entry_box, guint32 key)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
guint32 cbkey;

	model = gtk_combo_box_get_model(entry_box);
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
		gtk_tree_model_get (model, &iter, 1, &cbkey, -1);
		if(cbkey == key)
			break;

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	gtk_combo_box_set_active_iter(entry_box, &iter);

	return FALSE;
}




/**
 * ui_cur_combobox_add:
 *
 * Add a single element (useful for dynamics add)
 *
 * Return value: --
 *
 */
void
ui_cur_combobox_add(GtkComboBox *entry_box, Currency *cur)
{
	if( cur->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter  iter;

		model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, cur->name, 1, cur->key, -1);
	}
}

static void
ui_cur_combobox_populate_ghfunc(gpointer key, gpointer value, struct curPopContext *ctx)
{
GtkTreeIter  iter;
Currency *cur = value;

	if( ( cur->key != ctx->except_key ) )
	{
		gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter, 0, cur->name, 1, cur->key, -1);
	}
}

void
ui_cur_combobox_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint32 except_key)
{
GtkTreeModel *model;
struct curPopContext ctx;

    DB( g_print ("ui_cur_combobox_populate\n") );

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));

	/* keep our model alive and detach from combobox and completion */
	g_object_ref(model);
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), NULL);

	/* clear and populate */
	ctx.model = model;
	ctx.except_key = except_key;
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_hash_table_foreach(hash, (GHFunc)ui_cur_combobox_populate_ghfunc, &ctx);

	/* reatach our model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	g_object_unref(model);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

}

/**
 * ui_cur_combobox_populate:
 *
 * Populate the list and completion
 *
 * Return value: --
 *
 */
void
ui_cur_combobox_populate(GtkComboBox *entry_box, GHashTable *hash)
{
	ui_cur_combobox_populate_except(entry_box, hash, -1);
}


static gint
ui_cur_combobox_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint ret = 0;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, 0, &name1, -1);
    gtk_tree_model_get(model, b, 0, &name2, -1);

    if (name1 == NULL || name2 == NULL)
    {
        if (name1 == NULL && name2 == NULL)
        goto end;

        ret = (name1 == NULL) ? -1 : 1;
    }
    else
    {
        ret = g_utf8_collate(name1,name2);
    }


  end:

    g_free(name1);
    g_free(name2);

  	return ret;
  }


static void
ui_cur_combobox_test (GtkCellLayout   *cell_layout,
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
		g_object_set(cell, "text", _("(none)"), NULL);
	else
		g_object_set(cell, "text", name, NULL);

	//leak
	g_free(name);

}

/**
 * ui_cur_combobox_new:
 *
 * Create a new curee combobox
 *
 * Return value: the new widget
 *
 */
GtkWidget *
ui_cur_combobox_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *combobox;
GtkCellRenderer    *renderer;

	store = gtk_list_store_new (2,
		G_TYPE_STRING,
	    G_TYPE_INT
		);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_cur_combobox_compare_func, NULL, NULL);

	// dothe same for combobox

	combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));

	gtk_cell_layout_clear(GTK_CELL_LAYOUT (combobox));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (combobox),
					    renderer,
					    ui_cur_combobox_test,
					    NULL, NULL);

	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), combobox);

	gtk_widget_set_size_request (combobox, 10, -1);

	return combobox;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */




static void
ui_cur_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFCUR_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFCUR_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_cur_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint result = 0;
Currency *entry1, *entry2;
gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFCUR_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFCUR_DATAS, &entry2, -1);

	name1 = (entry1->key == GLOBALS->kcur) ? NULL : entry1->iso_code;
	name2 = (entry2->key == GLOBALS->kcur) ? NULL : entry2->iso_code;

    result = hb_string_compare(name1,name2);

    return result;
}

static void
ui_cur_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Currency *entry;
gchar *string;
gint weight;

	gtk_tree_model_get(model, iter, LST_DEFCUR_DATAS, &entry, -1);

	weight = entry->key == GLOBALS->kcur ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

	#if MYDEBUG
		if( entry->key == GLOBALS->kcur )
			string = g_strdup_printf ("[%d] %s - %s<span size=\"small\">\n(%s)</span>", entry->key, entry->iso_code, entry->name, _("Base currency"));
		else
			string = g_strdup_printf ("[%d] %s - %s", entry->key, entry->iso_code, entry->name);
		g_object_set(renderer, "weight", weight, "markup", string, NULL);
		g_free(string);
	#else
		if( entry->key == GLOBALS->kcur )
			string = g_strdup_printf ("%s - %s<span size=\"small\">\n(%s)</span>", entry->iso_code, entry->name, _("Base currency"));
		else
			string = g_strdup_printf ("%s - %s", entry->iso_code, entry->name);
		g_object_set(renderer, "weight", weight, "markup", string, NULL);
		g_free(string);
	#endif

}


static void
ui_cur_listview_symbol_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Currency *entry;
gint weight;

	gtk_tree_model_get(model, iter, LST_DEFCUR_DATAS, &entry, -1);

	weight = entry->key == GLOBALS->kcur ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
	
	g_object_set(renderer, "weight", weight, "text", entry->symbol, NULL);
}


static void
ui_cur_listview_lastmodified_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Currency *entry;
gchar buffer[256];
GDate date;
gint weight;

	gtk_tree_model_get(model, iter, LST_DEFCUR_DATAS, &entry, -1);

	weight = entry->key == GLOBALS->kcur ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
	
	if(entry->mdate > 0)
	{
		g_date_set_julian (&date, entry->mdate);
		g_date_strftime (buffer, 256-1, PREFS->date_format, &date);

		//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

		g_object_set(renderer, "weight", weight, "text", buffer, NULL);
    }
    else
		g_object_set(renderer, "weight", weight, "text", "-", NULL);
}



static void
ui_cur_listview_rate_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Currency *entry;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];

	gtk_tree_model_get(model, iter, LST_DEFCUR_DATAS, &entry, -1);

	if(entry->key == GLOBALS->kcur)
		g_object_set(renderer, "text", "-", NULL);
	else
	{
		//g_ascii_formatd(formatd_buf, sizeof (formatd_buf), "%.6f", entry->rate);
		//g_snprintf(formatd_buf, sizeof (formatd_buf), "%f", entry->rate);
		hb_str_rate(formatd_buf, sizeof (formatd_buf), entry->rate);
		g_object_set(renderer, "text", formatd_buf, NULL);
	}
	
}


/* = = = = = = = = = = = = = = = = */


void
ui_cur_listview_add(GtkTreeView *treeview, Currency *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFCUR_TOGGLE, FALSE,
			LST_DEFCUR_DATAS, item,
			-1);
	}
}

guint32
ui_cur_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Currency *item;

		gtk_tree_model_get(model, &iter, LST_DEFCUR_DATAS, &item, -1);

		if( item != NULL )
			return item->key;
	}
	return 0;
}

void
ui_cur_listview_remove_selected(GtkTreeView *treeview)
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


static void ui_cur_listview_populate_ghfunc(gpointer key, gpointer value, GtkTreeModel *model)
{
GtkTreeIter	iter;
Currency *item = value;

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
		LST_DEFCUR_TOGGLE	, FALSE,
		LST_DEFCUR_DATAS, item,
		-1);
}

void ui_cur_listview_populate(GtkWidget *view)
{
GtkTreeModel *model;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	g_hash_table_foreach(GLOBALS->h_cur, (GHFunc)ui_cur_listview_populate_ghfunc, model);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}

/* test */
/*
static void
ui_cur_listivew_rate_edited_func (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;

	g_print("cell edited '%s' path %s\n", new_text, path_string);

	
  gint column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (cell), "column"));

  gtk_tree_model_get_iter (model, &iter, path);

	Currency *item;

		gtk_tree_model_get(model, &iter, LST_DEFCUR_DATAS, &item, -1);

	item->rate = atof(new_text);

	GLOBALS->changes_count++;
	
	
  gtk_tree_path_free (path);
}
*/


GtkWidget *
ui_cur_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(
		NUM_LST_DEFCUR,
		G_TYPE_BOOLEAN,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	// column 1: toggle
	if( withtoggle == TRUE )
	{
		renderer = gtk_cell_renderer_toggle_new ();
		column = gtk_tree_view_column_new_with_attributes (_("Visible"),
							     renderer,
							     "active", LST_DEFCUR_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_cur_listview_toggled_cb), store);

	}

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);


	// column 1: name
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Name"));
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cur_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFCUR_DATAS), NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column 2: code
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.5, NULL);
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Symbol"));
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cur_listview_symbol_cell_data_function, GINT_TO_POINTER(LST_DEFCUR_DATAS), NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// column 3: base rate
	renderer = gtk_cell_renderer_text_new ();
	//g_object_set (renderer, "editable", TRUE, NULL);
	g_object_set(renderer, "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Exchange rate"));
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cur_listview_rate_cell_data_function, GINT_TO_POINTER(LST_DEFCUR_DATAS), NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	//g_signal_connect (renderer, "edited", G_CALLBACK (ui_cur_listivew_rate_edited_func), GTK_TREE_MODEL(store));

	// column 4: last modified
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 0.5, NULL);
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Last modfied"));
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_cur_listview_lastmodified_cell_data_function, GINT_TO_POINTER(LST_DEFCUR_DATAS), NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	
	// treeview attribute
	gtk_tree_view_set_search_column(GTK_TREE_VIEW(treeview), 1);
	//gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), TRUE);
	//gtk_tree_view_set_reorderable (GTK_TREE_VIEW(view), TRUE);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DEFCUR_DATAS, ui_cur_listview_compare_func, NULL, NULL);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DEFCUR_DATAS, GTK_SORT_ASCENDING);

	return treeview;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/*
** update the number sample label
*/
static void ui_cur_edit_dialog_update_sample(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_edit_dialog_data *data;
Currency cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar  buf[128];

	DB( g_printf("[ui_cur_edit] update sample\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	cur.symbol = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_symbol));
	cur.sym_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_symisprefix));
	cur.decimal_char  = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_decimalchar));
	cur.grouping_char = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_groupingchar));
	cur.frac_digits   = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_fracdigits));

	da_cur_initformat (&cur);
	DB( g_print("fmt: %s\n", cur.format) );
	
	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur.format, HB_NUMBER_SAMPLE);
	
	hb_str_formatd(buf, 127, formatd_buf, &cur, TRUE);
	gtk_label_set_text(GTK_LABEL(data->LB_sample), buf);

}



static void ui_cur_edit_dialog_set(GtkWidget *widget, Currency *cur)
{
struct ui_cur_edit_dialog_data *data;
Currency *base;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar label[128];

	DB( g_printf("[ui_cur_edit] set\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	g_snprintf(label, 127, "%s - %s", cur->iso_code, cur->name);
	gtk_label_set_text (GTK_LABEL(data->LB_name), label);

	base = da_cur_get(GLOBALS->kcur);
	
	g_snprintf(label, 127, "1 %s _=", base->iso_code);
	gtk_label_set_text_with_mnemonic (GTK_LABEL(data->LB_rate), label);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_rate), cur->rate);

	da_cur_initformat(cur);
	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur->format, HB_NUMBER_SAMPLE);
	hb_str_formatd(label, 127, formatd_buf, cur, TRUE);
	gtk_label_set_text (GTK_LABEL(data->LB_sample), label);

	ui_gtk_entry_set_text(data->ST_symbol, cur->symbol);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_symisprefix), cur->sym_prefix);
	ui_gtk_entry_set_text(data->ST_decimalchar, cur->decimal_char);
	ui_gtk_entry_set_text(data->ST_groupingchar, cur->grouping_char);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->NB_fracdigits), cur->frac_digits);

}



static void ui_cur_edit_dialog_get(GtkWidget *widget, Currency *cur)
{
struct ui_cur_edit_dialog_data *data;
gdouble rate;

	DB( g_printf("[ui_cur_edit] get\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	ui_gtk_entry_replace_text(data->ST_symbol, &cur->symbol);
	cur->sym_prefix = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_symisprefix));
	ui_gtk_entry_replace_text(data->ST_decimalchar, &cur->decimal_char);
	ui_gtk_entry_replace_text(data->ST_groupingchar, &cur->grouping_char);
	cur->frac_digits = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_fracdigits));

	da_cur_initformat(cur);

	rate = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->NB_rate));
	if(cur->rate != rate)
	{
		cur->rate = rate;
		cur->mdate = GLOBALS->today;
	}

}


void ui_cur_edit_dialog_new(GtkWindow *parent, Currency *cur)
{
struct ui_cur_edit_dialog_data data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *label, *widget, *expander;
gint crow, row;

	dialog = gtk_dialog_new_with_buttons (
	                _("Edit currency"),
				    GTK_WINDOW (parent),
				    0,
				    _("_Cancel"),
				    GTK_RESPONSE_REJECT,
				    _("_OK"),
				    GTK_RESPONSE_ACCEPT,
				    NULL);

	data.window = dialog;

	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_printf("[ui_cur_edit] new dialog=%p, inst_data=%p\n", dialog, &data) );


	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));	// return a vbox

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: Currency
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(_("Currency"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	widget = make_label(NULL, 0, 0.5);
	data.LB_name = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	// group :: exchange
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);
	
	label = make_label_group(_("Exchange rate"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);
	
	row = 1;
	label = make_label(NULL, 0, 0.5);
	data.LB_rate = label;
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_exchange_rate(label);
	data.NB_rate = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	gtk_widget_set_sensitive(group_grid, (GLOBALS->kcur == cur->key) ? FALSE : TRUE);

	// group :: format
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	label = make_label_group(_("Format"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 0, 0, 3, 1);

	row++;
	widget = make_label(NULL, 0, 0.5);
	data.LB_sample = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 2, 1);

	row++;
	expander = gtk_expander_new_with_mnemonic (_("_Customize"));
	gtk_grid_attach (GTK_GRID (group_grid), expander, 1, row, 2, 1);

    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_container_add (GTK_CONTAINER (expander), group_grid);

	row = 1;
	label = make_label_widget(_("_Symbol:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 3);
	data.ST_symbol = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("Is pre_fix"));
	data.CM_symisprefix = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Decimal char:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 1);
	data.ST_decimalchar = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

	row++;
	label = make_label_widget(_("_Frac digits:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_numeric(label, 0.0, 6.0);
	data.NB_fracdigits = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);
	
	row++;
	label = make_label_widget(_("_Grouping char:"));
	gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
	widget = make_string_maxlength(label, 1);
	data.ST_groupingchar = widget;
	gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);


	//gtk_window_resize(GTK_WINDOW(dialog), 400/PHI, 400);

	ui_cur_edit_dialog_set(dialog, cur);

	gtk_widget_show_all(content_area);

	//signals
    g_signal_connect (data.ST_symbol   , "changed", G_CALLBACK (ui_cur_edit_dialog_update_sample), NULL);
	g_signal_connect (data.CM_symisprefix, "toggled", G_CALLBACK (ui_cur_edit_dialog_update_sample), NULL);
	g_signal_connect (data.ST_decimalchar , "changed", G_CALLBACK (ui_cur_edit_dialog_update_sample), NULL);
    g_signal_connect (data.ST_groupingchar, "changed", G_CALLBACK (ui_cur_edit_dialog_update_sample), NULL);
    g_signal_connect (data.NB_fracdigits, "value-changed", G_CALLBACK (ui_cur_edit_dialog_update_sample), NULL);

	// wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_ACCEPT)
	{
		ui_cur_edit_dialog_get(dialog, cur);
	}

	// cleanup and destroy
	gtk_widget_destroy (dialog);

}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

enum {
	LST_CURSEL_NAME,
	LST_CURSEL_ISO,
	LST_CURSEL_FULLNAME,
	LST_CURSEL_DATA,
	NUM_LST_CURSEL
};


static void ui_cur_select_rowactivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
struct ui_cur_select_dialog_data *data = userdata;

	gtk_dialog_response(GTK_DIALOG(data->window), GTK_RESPONSE_ACCEPT);
}


static GtkTreeModel *ui_cur_select_model_create (void)
{
guint i = 0;
GtkListStore *store;
GtkTreeIter iter;
Currency4217 *cur;
gchar buffer[255];

	/* create list store */
	store = gtk_list_store_new (NUM_LST_CURSEL,
		G_TYPE_STRING, 
	    G_TYPE_STRING,
	    G_TYPE_STRING,
	    G_TYPE_POINTER,
	    NULL
	);

	for (i = 0; i< n_iso4217cur; i++)
	{
		cur = &iso4217cur[i];

		g_snprintf(buffer, 255-1, "%s - %s", cur->curr_iso_code, cur->name);
		
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
		    LST_CURSEL_NAME, cur->name,
		    LST_CURSEL_ISO, cur->curr_iso_code,
		    LST_CURSEL_FULLNAME, buffer,
		    LST_CURSEL_DATA, cur,
		    -1);


	}

	return GTK_TREE_MODEL (store);
}


static Currency4217 *ui_cur_select_dialog_get_langue(struct ui_cur_select_dialog_data *data)
{
GtkTreeSelection *treeselection;
gboolean selected;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
Currency4217 *curfmt = NULL;

	DB( g_printf("\n[ui_cur_select] get langue\n") );

	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_cur));
	selected = gtk_tree_selection_get_selected(treeselection, &model, &iter);
	if(selected)
	{
		gtk_tree_model_get(model, &iter, LST_CURSEL_DATA, &curfmt, -1);

		DB( g_printf(" - iso is '%s'\n", curfmt->curr_iso_code) );
	}	
	
	return curfmt;	
}


static void
ui_cur_select_search_changed_cb (GtkWidget *widget, gpointer user_data)
{
struct ui_cur_select_dialog_data *data = user_data;

	DB( g_printf("\n[ui_cur_select] search changed\n") );

	gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(data->modelfilter));
	
}


/* valid iso is empoty or 3 capital digit */
static guint currency_iso_code_valid(gchar *str)
{
guint n = 0;

	while( *str )
	{
		if( *str >= 'A' && *str <= 'Z' )
			n++;
		str++;
	}
	return n;
}


static void
ui_cur_select_custom_validate_cb(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_select_dialog_data *data = user_data;
gboolean custom;
gboolean valid = TRUE;
const gchar *iso, *name;
guint len;

	DB( g_printf("\n[ui_cur_select] custom validate\n") );
	
	custom = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_custom));

	DB( g_print(" custom=%d\n", custom) );

	//custom
	if( custom == TRUE )
	{
		valid = FALSE;
		
		name = gtk_entry_get_text (GTK_ENTRY (data->ST_custname));
		iso  = gtk_entry_get_text (GTK_ENTRY (data->ST_custiso));

		len = currency_iso_code_valid((gchar *)iso);

		DB( g_print(" name='%d', iso='%d'\n", (gint)strlen(name), len) );
	
		if( (len==0 || len==3) && (strlen(name) >= 3 ) )
		{
			valid = TRUE;
			// don't allow to enter stand 4217 iso code
			if( len == 3 )
			{
			Currency4217 *stdcur = iso4217format_get((gchar *)iso);
				if(stdcur != NULL)
					valid = FALSE;					
			}
		}
	}

	gtk_dialog_set_response_sensitive(GTK_DIALOG(data->window), GTK_RESPONSE_ACCEPT, valid);

}


static void
ui_cur_select_custom_activate_cb(GtkWidget *widget, gpointer     user_data)
{
struct ui_cur_select_dialog_data *data = user_data;
gboolean custom;

	DB( g_printf("\n[ui_cur_select] custom activate\n") );

	custom = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data->CM_custom));

	DB( g_print(" custom=%d\n", custom) );

	gtk_widget_set_sensitive(data->ST_search, !custom);
	gtk_widget_set_sensitive(data->LV_cur, !custom);

	hb_widget_visible (data->LB_custname, custom);
	hb_widget_visible (data->ST_custname, custom);
	hb_widget_visible (data->LB_custiso, custom);
	hb_widget_visible (data->ST_custiso, custom);

	if(custom)
	{
		gtk_tree_selection_unselect_all(gtk_tree_view_get_selection (GTK_TREE_VIEW(data->LV_cur)));
		gtk_window_set_focus(GTK_WINDOW(data->window), data->ST_custname);
	}

	ui_cur_select_custom_validate_cb(data->window, data);

}


static gboolean
ui_cur_select_model_func_visible (GtkTreeModel *model,
              GtkTreeIter  *iter,
              gpointer      data)
{
// Visible if row is non-empty and first column is “HI”
gchar *str;
gboolean visible = TRUE;
GtkEntry *entry = data;

	if(!GTK_IS_ENTRY(entry))
		return TRUE;

	gchar *needle = g_ascii_strdown(gtk_entry_get_text(entry), -1);

  gtk_tree_model_get (model, iter, LST_CURSEL_FULLNAME, &str, -1);

	gchar *haystack = g_ascii_strdown(str, -1);

	if (str && g_strrstr (haystack, needle) == NULL)
	{
		visible = FALSE;
	}

	DB( g_print("filter: '%s' '%s' %d\n", str, needle, visible) );

	g_free(haystack);
	g_free(needle);
  g_free (str);

  return visible;
}


gint ui_cur_select_dialog_new(GtkWindow *parent, gint select_mode, struct curSelectContext *ctx)
{
struct ui_cur_select_dialog_data data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid;
GtkWidget *scrollwin, *treeview, *label, *widget;
gint crow, row;

	memset(&data, 0, sizeof(struct ui_cur_select_dialog_data));

	dialog = gtk_dialog_new_with_buttons (
	                (select_mode == CUR_SELECT_MODE_BASE) ? _("Select base currency") : _("Select currency"),
				    GTK_WINDOW (parent),
				    0,
				    _("_Cancel"),
				    GTK_RESPONSE_REJECT,
				    _("_OK"),
				    GTK_RESPONSE_ACCEPT,
				    NULL);

	data.window = dialog;

	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_printf("\n[ui_cur_select] new dialog=%p, inst_data=%p\n", dialog, &data) );


	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));	// return a vbox

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: Search
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	row = 0;
	widget = gtk_search_entry_new();
	data.ST_search = widget;
	gtk_widget_set_hexpand (widget, TRUE);
	gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 4, 1);


	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_grid_attach (GTK_GRID (group_grid), scrollwin, 1, row, 4, 1);

	gtk_widget_set_vexpand (scrollwin, TRUE);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	//test treefilter
	data.model = ui_cur_select_model_create();
	data.modelfilter = gtk_tree_model_filter_new(GTK_TREE_MODEL(data.model), NULL);
	gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(data.modelfilter), ui_cur_select_model_func_visible, data.ST_search, NULL);
	data.sortmodel = gtk_tree_model_sort_new_with_model(data.modelfilter);
	
	//treeview
	treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(data.sortmodel));
	data.LV_cur = treeview;
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview), LST_CURSEL_NAME);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(data.sortmodel), LST_CURSEL_NAME, GTK_SORT_ASCENDING);
	//g_object_unref (model);
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);

	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	
	// populate list
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer, "text", LST_CURSEL_NAME, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_CURSEL_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("ISO Code"), renderer, "text", LST_CURSEL_ISO, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_CURSEL_ISO);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

	if( select_mode == CUR_SELECT_MODE_NORMAL )
	{
		// group :: Custom
		row++;
		widget = gtk_check_button_new_with_mnemonic (_("Add a custom _currency"));
		data.CM_custom = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 1, row, 4, 1);
		
		//custom currency (crypto and discontinued)
		row++;
		label = make_label_widget(_("_Name:"));
		data.LB_custname = label;
		gtk_grid_attach (GTK_GRID (group_grid), label, 1, row, 1, 1);
		widget = make_string(label);
		data.ST_custname = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 2, row, 1, 1);

		label = make_label_widget("_ISO:");
		data.LB_custiso = label;
		gtk_grid_attach (GTK_GRID (group_grid), label, 3, row, 1, 1);
		widget = make_string_maxlength(label, 3);
		data.ST_custiso = widget;
		gtk_grid_attach (GTK_GRID (group_grid), widget, 4, row, 1, 1);

		g_signal_connect (G_OBJECT (data.CM_custom)  , "toggled", G_CALLBACK (ui_cur_select_custom_activate_cb), &data);	
		g_signal_connect (G_OBJECT (data.ST_custname), "changed", G_CALLBACK (ui_cur_select_custom_validate_cb), &data);
		g_signal_connect (G_OBJECT (data.ST_custiso) , "changed", G_CALLBACK (ui_cur_select_custom_validate_cb), &data);
	
	}

	gtk_window_resize(GTK_WINDOW(dialog), 400/PHI, 400);


	gtk_widget_show_all(content_area);
	hb_widget_visible (data.LB_custname, FALSE);
	hb_widget_visible (data.ST_custname, FALSE);
	hb_widget_visible (data.LB_custiso, FALSE);
	hb_widget_visible (data.ST_custiso, FALSE);


	// signals
	g_signal_connect (G_OBJECT(data.ST_search), "search-changed", G_CALLBACK (ui_cur_select_search_changed_cb), &data);
	g_signal_connect (G_OBJECT(data.LV_cur), "row-activated", G_CALLBACK (ui_cur_select_rowactivated), &data);

	//init picker struct
	memset(ctx, 0, sizeof(struct curSelectContext));

	// wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if(result == GTK_RESPONSE_ACCEPT)
	{
	gboolean custom;
	
		custom = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(data.CM_custom));
		if(!custom)
		{
			ctx->cur_4217 = ui_cur_select_dialog_get_langue(&data);
		}
		else
		//never fill custom in base mode
		if( select_mode != CUR_SELECT_MODE_BASE )
		{
			ctx->cur_name = g_strdup(gtk_entry_get_text (GTK_ENTRY(data.ST_custname)));
			ctx->cur_iso = g_strdup(gtk_entry_get_text (GTK_ENTRY(data.ST_custiso)));
		}
	}

	// cleanup and destroy
	gtk_widget_destroy (dialog);

	return result;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


gint ui_cur_manage_dialog_update_currencies(GtkWindow *parent)
{
GError *error = NULL;
gboolean retcode = FALSE;

	DB( g_printf("\n[ui_cur_manage] update currencies\n") );

	// do nothing if just the base currency
	if(da_cur_length() <= 1)
		return TRUE;

	retcode = currency_online_sync(&error);
	
	DB( g_print("retcode: %d\n", retcode) );

	if(!retcode)
	{
	gchar *msg = _("Unknow error");

		if( error )
			msg = error->message;

		g_warning("update online: '%s'", msg);

		ui_dialog_msg_infoerror(GTK_WINDOW(parent), GTK_MESSAGE_ERROR,
			_("Update online error"),
			msg,
			NULL
			);
	
		if( error )
			g_error_free (error);
	}
	
	return retcode;
}


static void
ui_cur_manage_dialog_sync(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
gboolean retcode;
	
	DB( g_printf("\n[ui_cur_manage] sync online\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	retcode = ui_cur_manage_dialog_update_currencies(GTK_WINDOW(data->window));

	if(retcode == TRUE)
	{
		gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_cur));
		//todo: (or not) msg with changes
		
	}
}


/*
**
*/
static void ui_cur_manage_dialog_update(GtkWidget *treeview, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
GtkTreeSelection *selection;
GtkTreeModel *model;
GtkTreeIter iter;
Currency *item;
gboolean sensitive;

	DB( g_printf("\n[ui_cur_manage] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	sensitive = da_cur_length() <= 1 ? FALSE : TRUE;
	gtk_widget_set_sensitive (data->BB_update, sensitive);


	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cur));

	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFCUR_DATAS, &item, -1);

		gtk_widget_set_sensitive(data->BT_edit, TRUE);

		sensitive = !(currency_is_used(item->key));
		//gtk_widget_set_sensitive(data->BT_mov, sensitive);
		//gtk_widget_set_sensitive(data->BT_mod, sensitive);
		gtk_widget_set_sensitive(data->BT_rem, sensitive);

		//disable set as base on actual base currency
		//disable on custom currency
		sensitive = TRUE;
		if( (item->key == GLOBALS->kcur) || (item->flags & CF_CUSTOM) )
			sensitive = FALSE; 
		gtk_widget_set_sensitive(data->BT_base, sensitive);
	}
	else
	{
		gtk_widget_set_sensitive(data->BT_edit, FALSE);
		gtk_widget_set_sensitive(data->BT_rem , FALSE);
		gtk_widget_set_sensitive(data->BT_base, FALSE);
	}
}


/**
 * ui_cur_manage_dialog_add:
 *
 */
static void
ui_cur_manage_dialog_add(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
struct curSelectContext selectCtx;
gint result;
gboolean added;

	DB( g_printf("\n[ui_cur_manage] add\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	result = ui_cur_select_dialog_new(GTK_WINDOW(data->window), CUR_SELECT_MODE_NORMAL, &selectCtx);
	if( result == GTK_RESPONSE_ACCEPT )
	{
	Currency *item = NULL;

		added = FALSE;
		if( selectCtx.cur_4217 != NULL )
		{
		Currency4217 *curfmt;

			curfmt = selectCtx.cur_4217;

			DB( g_printf("- user selected: '%s' '%s'\n", curfmt->curr_iso_code, curfmt->name) );
			item = da_cur_get_by_iso_code(curfmt->curr_iso_code);
			if( item == NULL )
			{
				item = currency_add_from_user(curfmt);
				added = TRUE;
			}
		}
		else
		{		
			DB( g_printf("- user custom: '%s' '%s'\n", selectCtx.cur_iso, selectCtx.cur_name) );

			item = da_cur_malloc ();
			item->flags |= CF_CUSTOM;
			item->name = g_strdup(selectCtx.cur_name);
			item->iso_code = g_strdup(selectCtx.cur_iso);
			item->symbol = g_strdup(item->iso_code);
			item->frac_digits = 2;
			item->sym_prefix = FALSE;
			item->decimal_char = g_strdup(".");
			item->grouping_char = NULL;
			
			added = da_cur_append(item);
			if( !added )
			{
				//not append (duplicate)
				da_cur_free (item);
				item = NULL;
			}

			g_free(selectCtx.cur_iso);
			g_free(selectCtx.cur_name);
		}

		if( added )
		{
			ui_cur_listview_add(GTK_TREE_VIEW(data->LV_cur), item);
			gtk_tree_sortable_sort_column_changed(GTK_TREE_SORTABLE(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_cur))));
			
			ui_cur_manage_dialog_update (widget, user_data);
			GLOBALS->changes_count++;
		}

	}
}


static void
ui_cur_manage_dialog_modify(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_printf("\n[ui_cur_manage] modify\n") );

	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_cur));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Currency *item;

		gtk_tree_model_get(model, &iter, LST_DEFCUR_DATAS, &item, -1);

		if( item!= NULL	 )
		{
			ui_cur_edit_dialog_new(GTK_WINDOW(data->window), item);
			GLOBALS->changes_count++;
		}

	}
}


/*
** remove the selected curee to our treeview and temp GList
*/
static void ui_cur_manage_dialog_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
guint32 key;
gboolean do_remove, result;

	DB( g_printf("\n[ui_cur_manage] remove\n") );
	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	
	do_remove = TRUE;
	key = ui_cur_listview_get_selected_key(GTK_TREE_VIEW(data->LV_cur));
	if( key > 0 )
	{
	Currency *cur;
	gchar *title;
	gchar *secondtext;

		if( currency_is_used(key) == TRUE )
		{
			do_remove = FALSE;
		}
		else
		{
			cur = da_cur_get(key);
		
			title = g_strdup_printf (
				_("Are you sure you want to permanently delete '%s'?"), cur->name);

			secondtext = _("If you delete a currency, it will be permanently lost.");
		
			result = ui_dialog_msg_confirm_alert(
					GTK_WINDOW(data->window),
					title,
					secondtext,
					_("_Delete")
				);

			g_free(title);

			do_remove = (result == GTK_RESPONSE_OK) ? TRUE :FALSE;
		}	
		
		if( do_remove )
		{
			da_cur_remove(key);
			ui_cur_listview_remove_selected(GTK_TREE_VIEW(data->LV_cur));
			ui_cur_manage_dialog_update (widget, user_data);
			data->change++;
		}
	}

}

/*
** button callback: set base currency
*/
static void ui_cur_manage_dialog_setbase(GtkWidget *widget, gpointer user_data)
{
struct ui_cur_manage_dialog_data *data;
guint32 key;
gboolean do_change;

	DB( g_printf("\n[ui_cur_manage] setbase\n") );

	
	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	key = ui_cur_listview_get_selected_key(GTK_TREE_VIEW(data->LV_cur));
	if( key > 0 )
	{
		do_change = ui_dialog_msg_question(
			GTK_WINDOW(data->window),
			_("Change the base currency"),
			_("If you proceed, rates of other currencies\n"
			  "will be set to 0, don't forget to update it"),
			NULL
			);
		if(do_change == GTK_RESPONSE_YES)
		{
			hbfile_change_basecurrency(key);
			gtk_tree_view_columns_autosize(GTK_TREE_VIEW(data->LV_cur));
		}
	}

}




/*
**
*/
static void ui_cur_manage_dialog_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_cur_manage_dialog_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

static void ui_cur_manage_dialog_onRowActivated (GtkTreeView        *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer            userdata)
{

	//model = gtk_tree_view_get_model(treeview);
	//gtk_tree_model_get_iter_first(model, &iter);
	//if(gtk_tree_selection_iter_is_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter) == FALSE)
	//{
		ui_cur_manage_dialog_modify(GTK_WIDGET(treeview), NULL);
	//}
}



/*
**
*/
static void ui_cur_manage_dialog_setup(struct ui_cur_manage_dialog_data *data)
{

	DB( g_printf("\n[ui_cur_manage] setup\n") );


	ui_cur_listview_populate(data->LV_cur);

	//ui_cur_combobox_populate(data->CY_curr, GLOBALS->h_cur);
	//ui_cur_combobox_set_active(GTK_COMBO_BOX(data->CY_curr), GLOBALS->kcur);



}


/*
**
*/
GtkWidget *ui_cur_manage_dialog (void)
{
struct ui_cur_manage_dialog_data data;
GtkWidget *dialog, *content_area, *content_grid, *group_grid, *bbox;
GtkWidget *widget, *scrollwin, *treeview;
gint crow, row, w, h;

	dialog = gtk_dialog_new_with_buttons (_("Currencies"),
					    GTK_WINDOW(GLOBALS->mainwindow),
					    0,
					    _("_Close"),
					    GTK_RESPONSE_ACCEPT,
					    NULL);

	data.window = dialog;
	data.change = 0;

	//set the dialog icon
	gtk_window_set_icon_name(GTK_WINDOW (dialog), ICONNAME_HB_CURRENCY);

	//set a nice dialog size
	gtk_window_get_size(GTK_WINDOW(GLOBALS->mainwindow), &w, &h);
	gtk_window_set_default_size (GTK_WINDOW(dialog), -1, h/PHI);

	
	//store our window private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_printf("[ui_cur_manage] new dialog=%p, inst_data=%p\n", dialog, &data) );

    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);

	content_area = gtk_dialog_get_content_area(GTK_DIALOG (dialog));	// return a vbox

	content_grid = gtk_grid_new();
	gtk_grid_set_row_spacing (GTK_GRID (content_grid), SPACING_LARGE);
	gtk_orientable_set_orientation(GTK_ORIENTABLE(content_grid), GTK_ORIENTATION_VERTICAL);
	gtk_container_set_border_width (GTK_CONTAINER(content_grid), SPACING_MEDIUM);
	gtk_box_pack_start (GTK_BOX (content_area), content_grid, TRUE, TRUE, 0);

	crow = 0;
	// group :: --------
    group_grid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (group_grid), SPACING_SMALL);
	gtk_grid_set_column_spacing (GTK_GRID (group_grid), SPACING_MEDIUM);
	gtk_grid_attach (GTK_GRID (content_grid), group_grid, 0, crow++, 1, 1);

	row = 1;
	bbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING_MEDIUM);
	data.BB_update = bbox;
	gtk_grid_attach (GTK_GRID(group_grid), bbox, 0, row, 1, 1);

	widget = gtk_button_new_from_icon_name (ICONNAME_REFRESH, GTK_ICON_SIZE_BUTTON);
	gtk_container_add (GTK_CONTAINER (bbox), widget);
	
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (ui_cur_manage_dialog_sync), NULL);

	widget = make_label_widget (_("Update online"));
	gtk_container_add (GTK_CONTAINER (bbox), widget);
	
	row++;
	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrollwin), HB_MINHEIGHT_LIST);
	treeview = ui_cur_listview_new(FALSE);
 	data.LV_cur = treeview;
	gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
	gtk_widget_set_vexpand (scrollwin, TRUE);
	gtk_widget_set_hexpand (scrollwin, TRUE);
	gtk_grid_attach (GTK_GRID(group_grid), scrollwin, 0, row, 2, 1);

	row++;
	bbox = gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_START);
	gtk_box_set_spacing (GTK_BOX (bbox), SPACING_SMALL);
	gtk_grid_attach (GTK_GRID (group_grid), bbox, 0, row, 2, 1);

	widget = gtk_button_new_with_mnemonic(_("_Add"));
	data.BT_add = widget;
	gtk_container_add (GTK_CONTAINER (bbox), widget);

	widget = gtk_button_new_with_mnemonic(_("_Edit"));
	data.BT_edit = widget;
	gtk_container_add (GTK_CONTAINER (bbox), widget);

	widget = gtk_button_new_with_mnemonic(_("_Delete"));
	data.BT_rem = widget;
	gtk_container_add (GTK_CONTAINER (bbox), widget);

	widget = gtk_button_new_with_mnemonic(_("Set as base"));
	data.BT_base = widget;
	gtk_container_add (GTK_CONTAINER (bbox), widget);


	//connect all our signals
	//g_signal_connect (G_OBJECT (data.ST_name), "activate", G_CALLBACK (ui_cur_manage_dialog_add), NULL);

	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_cur)), "changed", G_CALLBACK (ui_cur_manage_dialog_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_cur), "row-activated", G_CALLBACK (ui_cur_manage_dialog_onRowActivated), NULL);

	g_signal_connect (G_OBJECT (data.BT_add), "clicked", G_CALLBACK (ui_cur_manage_dialog_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_cur_manage_dialog_remove), NULL);
	g_signal_connect (G_OBJECT (data.BT_edit), "clicked", G_CALLBACK (ui_cur_manage_dialog_modify), NULL);
	//g_signal_connect (G_OBJECT (data.BT_mov), "clicked", G_CALLBACK (ui_cur_manage_dialog_move), NULL);
	g_signal_connect (G_OBJECT (data.BT_base), "clicked", G_CALLBACK (ui_cur_manage_dialog_setbase), NULL);

	//setup, init and show window
	ui_cur_manage_dialog_setup(&data);

	ui_cur_manage_dialog_update(data.LV_cur, NULL);

	//gtk_window_resize(GTK_WINDOW(dialog), 256, 414);

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
	GLOBALS->changes_count += data.change;
	gtk_widget_destroy (dialog);

	return NULL;
}



