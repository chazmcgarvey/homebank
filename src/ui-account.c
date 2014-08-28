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
#include "hb-account.h"
#include "ui-account.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;


gchar *CYA_ACC_TYPE[] = 
{
	N_("(no type)"),
	N_("Bank"),
	N_("Cash"),
	N_("Asset"),
	N_("Credit card"),
	N_("Liability"),
	NULL
};


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * ui_acc_comboboxentry_get_name:
 * 
 * get the name of the active account or -1
 * 
 * Return value: a new allocated name tobe freed with g_free
 *
 */
gchar *
ui_acc_comboboxentry_get_name(GtkComboBox *entry_box)
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
 * ui_acc_comboboxentry_get_key:
 * 
 * get the key of the active account
 * and create the account if it do not exists
 * 
 * Return value: the key or 0
 *
 */
guint32
ui_acc_comboboxentry_get_key(GtkComboBox *entry_box)
{
gchar *name;
Account *item;

	name = ui_acc_comboboxentry_get_name(entry_box);

	item = da_acc_get_by_name(name);

	g_free(name);

	if( item == NULL )
	{
		//todo: ask the user here		
		/*		
		item = da_acc_malloc();
		item->name = g_strdup(name);
		da_acc_insert(item);
		ui_acc_comboboxentry_add(entry_box, item);
		*/
		return 0;
	}



	return item->key;
}

gboolean
ui_acc_comboboxentry_set_active(GtkComboBox *entry_box, guint32 key)
{
Account *item;
	
	if( key > 0 )
	{	
		item = da_acc_get(key);
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
 * ui_acc_comboboxentry_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_acc_comboboxentry_add(GtkComboBox *entry_box, Account *acc)
{
	if( acc->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter  iter;

		model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));
	
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, acc->name,
			1, acc->pos,
			-1);
	}
}


static void
ui_acc_comboboxentry_populate_ghfunc(gpointer key, gpointer value, struct accPopContext *ctx)
{
GtkTreeIter  iter;
Account *acc = value;

	if( (acc->flags & AF_CLOSED) ) return;
	if( (ctx->insert_type == ACC_LST_INSERT_REPORT) && (acc->flags & AF_NOREPORT) ) return;
	if( (acc->key == ctx->except_key) ) return;
	if( (acc->imported == TRUE) ) return;
	
    DB( g_print (" -> append %s\n", acc->name) );
	
	
	gtk_list_store_append (GTK_LIST_STORE(ctx->model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(ctx->model), &iter,
			0, acc->name,
			1, acc->pos,
			-1);
}

/**
 * ui_acc_comboboxentry_populate:
 * 
 * Populate the list and completion
 * 
 * Return value: --
 *
 */
void
ui_acc_comboboxentry_populate(GtkComboBox *entry_box, GHashTable *hash, gint insert_type)
{
	ui_acc_comboboxentry_populate_except(entry_box, hash, 0, insert_type);
}

void
ui_acc_comboboxentry_populate_except(GtkComboBox *entry_box, GHashTable *hash, guint except_key, gint insert_type)
{
GtkTreeModel *model;
GtkEntryCompletion *completion;
struct accPopContext ctx;

    DB( g_print ("ui_acc_comboboxentry_populate\n") );

    DB( g_print (" -> except is %d\n", except_key) );

	model = gtk_combo_box_get_model(GTK_COMBO_BOX(entry_box));
	completion = gtk_entry_get_completion(GTK_ENTRY (gtk_bin_get_child(GTK_BIN (entry_box))));
	
	/* keep our model alive and detach from comboboxentry and completion */
	g_object_ref(model);
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), NULL);
	gtk_entry_completion_set_model (completion, NULL);

	/* clear and populate */
	ctx.model = model;
	ctx.except_key = except_key;
	ctx.kcur = 0;
	ctx.insert_type = insert_type;
/*	Account *acc = da_acc_get(except_key);
	if(acc != NULL)
		ctx.kcur = acc->kcur;*/
	
	gtk_list_store_clear (GTK_LIST_STORE(model));
	g_hash_table_foreach(hash, (GHFunc)ui_acc_comboboxentry_populate_ghfunc, &ctx);

	/* reatach our model */
	gtk_combo_box_set_model(GTK_COMBO_BOX(entry_box), model);
	gtk_entry_completion_set_model (completion, model);
	g_object_unref(model);

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(model), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	
}


static gint
ui_acc_comboboxentry_compare_func (GtkTreeModel *model, GtkTreeIter  *a, GtkTreeIter  *b, gpointer      userdata)
{
gint pos1, pos2;
  
    gtk_tree_model_get(model, a, 1, &pos1, -1);
    gtk_tree_model_get(model, b, 1, &pos2, -1);
  	return (pos1 - pos2);
}


/**
 * ui_acc_comboboxentry_new:
 * 
 * Create a new account comboboxentry
 * 
 * Return value: the new widget
 *
 */
GtkWidget *
ui_acc_comboboxentry_new(GtkWidget *label)
{
GtkListStore *store;
GtkWidget *comboboxentry;
GtkEntryCompletion *completion;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_comboboxentry_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

    completion = gtk_entry_completion_new ();
    gtk_entry_completion_set_model (completion, GTK_TREE_MODEL(store));
    gtk_entry_completion_set_text_column (completion, 0);

/*	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (completion), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (completion), renderer, "text", 0, NULL);
*/

	comboboxentry = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(store));
	gtk_entry_set_completion (GTK_ENTRY (gtk_bin_get_child(GTK_BIN (comboboxentry))), completion);
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(comboboxentry), 0);

/*	gtk_cell_layout_clear(GTK_CELL_LAYOUT (comboboxentry));

	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (comboboxentry), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (comboboxentry), renderer, "text", 0, NULL);
*/
	g_object_unref(store);

	if(label)
		gtk_label_set_mnemonic_widget (GTK_LABEL(label), comboboxentry);

	gtk_widget_set_size_request(comboboxentry, HB_MINWIDTH_COMBO, -1);
	
	return comboboxentry;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


static void
ui_acc_listview_toggled_cb (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_DEFACC_TOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_DEFACC_TOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

static gint
ui_acc_listview_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata)
{
gint retval = 0;
Account *entry1, *entry2;
//gchar *name1, *name2;

    gtk_tree_model_get(model, a, LST_DEFACC_DATAS, &entry1, -1);
    gtk_tree_model_get(model, b, LST_DEFACC_DATAS, &entry2, -1);

	retval = entry1->pos - entry2->pos;

    return retval;
}

static void
ui_acc_listview_name_cell_data_function (GtkTreeViewColumn *col,
				GtkCellRenderer *renderer,
				GtkTreeModel *model,
				GtkTreeIter *iter,
				gpointer user_data)
{
Account *entry;
gchar *name;
#if MYDEBUG
gchar *string;
#endif

	gtk_tree_model_get(model, iter, LST_DEFACC_DATAS, &entry, -1);
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
 * acc_list_add:
 * 
 * Add a single element (useful for dynamics add)
 * 
 * Return value: --
 *
 */
void
ui_acc_listview_add(GtkTreeView *treeview, Account *item)
{
	if( item->name != NULL )
	{
	GtkTreeModel *model;
	GtkTreeIter	iter;

		model = gtk_tree_view_get_model(treeview);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFACC_TOGGLE, FALSE,
			LST_DEFACC_DATAS, item,
			-1);

		gtk_tree_selection_select_iter (gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), &iter);

	}
}

guint32
ui_acc_listview_get_selected_key(GtkTreeView *treeview)
{
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	selection = gtk_tree_view_get_selection(treeview);
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Account *item;

		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);
		
		if( item!= NULL	 )
			return item->key;
	}
	return 0;
}

void
ui_acc_listview_remove_selected(GtkTreeView *treeview)
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


static gint ui_acc_glist_compare_func(Account *a, Account *b)
{
	return ((gint)a->pos - b->pos);
}


void ui_acc_listview_populate(GtkWidget *view, gint insert_type)
{
GtkTreeModel *model;
GtkTreeIter	iter;
GList *lacc, *list;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */

	/* populate */
	//g_hash_table_foreach(GLOBALS->h_acc, (GHFunc)ui_acc_listview_populate_ghfunc, model);
	list = g_hash_table_get_values(GLOBALS->h_acc);
	
	lacc = list = g_list_sort(list, (GCompareFunc)ui_acc_glist_compare_func);
	while (list != NULL)
	{
	Account *item = list->data;
	
		if( insert_type == ACC_LST_INSERT_REPORT )
		{
			if( (item->flags & AF_CLOSED) ) goto next1;
			if( (item->flags & AF_NOREPORT) ) goto next1;
		}
		
		DB( g_print(" populate: %d\n", item->key) );

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			LST_DEFACC_TOGGLE	, FALSE,
			LST_DEFACC_DATAS, item,
			-1);

next1:
		list = g_list_next(list);
	}
	g_list_free(lacc);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
	g_object_unref(model);
}


GtkWidget *
ui_acc_listview_new(gboolean withtoggle)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	// create list store
	store = gtk_list_store_new(NUM_LST_DEFACC,
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
							     "active", LST_DEFACC_TOGGLE,
							     NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		g_signal_connect (renderer, "toggled",
			    G_CALLBACK (ui_acc_listview_toggled_cb), store);

	}

	// column 2: name
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ui_acc_listview_name_cell_data_function, GINT_TO_POINTER(LST_DEFACC_DATAS), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);
	
	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_listview_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/*
** get widgets contents to the selected account
*/
/*
static void ui_acc_manage_get(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gchar *txt;
gboolean bool;
gdouble value;

Account *item;

gint field = GPOINTER_TO_INT(user_data);

	DB( g_print("(ui_acc_manage_) get %d\n", field) );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		data->change++;

		switch( field )
		{
			case FIELD_NAME:
				txt = (gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_name));
				// ignore if entry is empty
				if (txt && *txt)
				{
					bool = account_rename(item, txt);					
					if(bool)
					{
						gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
					}
					else
					{
						gtk_entry_set_text(GTK_ENTRY(data->ST_name), item->name);
					}
				}
				break;

		//todo: for stock account			
		
			//case FIELD_TYPE:
			//	item->type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));
			//	break;
		
			case FIELD_BANK:
				g_free(item->bankname);
				item->bankname = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_bank)));
				break;

			case FIELD_NUMBER:
				g_free(item->number);
				item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));
				break;

			case FIELD_BUDGET:
				item->flags &= ~(AF_BUDGET);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_nobudget));
				if(bool) item->flags |= AF_BUDGET;
				break;

			case FIELD_CLOSED:
				item->flags &= ~(AF_CLOSED);
				bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_closed));
				if(bool) item->flags |= AF_CLOSED;
				break;

			case FIELD_INITIAL:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
				item->initial = value;
				break;

			case FIELD_MINIMUM:
				value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
				item->minimum = value;
				break;

			case FIELD_CHEQUE1:
				item->cheque1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque1));
				break;

			case FIELD_CHEQUE2:
				item->cheque2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque2));
				break;
		}
	}

}
*/


static gchar *dialog_get_name(gchar *title, gchar *origname, GtkWindow *parentwindow)
{
GtkWidget *dialog, *content, *mainvbox, *getwidget;
gchar *retval = NULL;
	
		dialog = gtk_dialog_new_with_buttons (title,
						    GTK_WINDOW (parentwindow),
						    0,
						    GTK_STOCK_CANCEL,
						    GTK_RESPONSE_REJECT,
						    GTK_STOCK_OK,
						    GTK_RESPONSE_ACCEPT,
						    NULL);

		content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
	
		mainvbox = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (content), mainvbox, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (mainvbox), HB_BOX_SPACING);

		getwidget = gtk_entry_new();
		gtk_box_pack_start (GTK_BOX (mainvbox), getwidget, TRUE, TRUE, 0);
		gtk_widget_show_all(mainvbox);

		if(origname != NULL)
			gtk_entry_set_text(GTK_ENTRY(getwidget), origname);
		gtk_widget_grab_focus (getwidget);

		gtk_entry_set_activates_default (GTK_ENTRY(getwidget), TRUE);

		gtk_dialog_set_default_response(GTK_DIALOG( dialog ), GTK_RESPONSE_ACCEPT);

		//wait for the user
		gint result = gtk_dialog_run (GTK_DIALOG (dialog));

		if(result == GTK_RESPONSE_ACCEPT)
		{
		const gchar *name;

			name = gtk_entry_get_text(GTK_ENTRY(getwidget));

			/* ignore if entry is empty */
			if (name && *name)
			{
				retval = g_strdup(name);
			}
	    }

		// cleanup and destroy
		gtk_widget_destroy (dialog);
	

	return retval;
}





static void ui_acc_manage_getlast(struct ui_acc_manage_data *data)
{
gboolean bool;
gdouble value;
Account *item;

	DB( g_print("\n(ui_acc_manage_getlast)\n") );

	DB( g_print(" -> for account id=%d\n", data->lastkey) );

	item = da_acc_get(data->lastkey);
	if(item != NULL)
	{	
		data->change++;

		item->type = gtk_combo_box_get_active(GTK_COMBO_BOX(data->CY_type));

		//account_set_currency(item, ui_cur_combobox_get_key(GTK_COMBO_BOX(data->CY_curr)) );

		g_free(item->bankname);
		item->bankname = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_bank)));

		g_free(item->number);
		item->number = g_strdup(gtk_entry_get_text(GTK_ENTRY(data->ST_number)));

		item->flags &= ~(AF_CLOSED);
		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_closed));
		if(bool) item->flags |= AF_CLOSED;

		item->flags &= ~(AF_NOBUDGET);
		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_nobudget));
		if(bool) item->flags |= AF_NOBUDGET;

		item->flags &= ~(AF_NOSUMMARY);
		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_nosummary));
		if(bool) item->flags |= AF_NOSUMMARY;

		item->flags &= ~(AF_NOREPORT);
		bool = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->CM_noreport));
		if(bool) item->flags |= AF_NOREPORT;

		gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_initial));
		value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_initial));
		item->initial = value;

		gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_minimum));
		value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_minimum));
		item->minimum = value;

		gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_cheque1));
		item->cheque1 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque1));

		gtk_spin_button_update(GTK_SPIN_BUTTON(data->ST_cheque2));
		item->cheque2 = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->ST_cheque2));

		
	}

}



/*
** set widgets contents from the selected account
*/
static void ui_acc_manage_set(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
Account *item;

	DB( g_print("\n(ui_acc_manage_set)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc));
	//if true there is a selected node
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, LST_DEFACC_DATAS, &item, -1);

		DB( g_print(" -> set acc id=%d\n", item->key) );

		gtk_combo_box_set_active(GTK_COMBO_BOX(data->CY_type), item->type );

		//ui_cur_combobox_set_active(GTK_COMBO_BOX(data->CY_curr), item->kcur);
		
		if(item->bankname != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), item->bankname);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_bank), "");

		if(item->number != NULL)
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), item->number);
		else
			gtk_entry_set_text(GTK_ENTRY(data->ST_number), "");


		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_nobudget), item->flags & AF_NOBUDGET);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_nosummary), item->flags & AF_NOSUMMARY);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_noreport), item->flags & AF_NOREPORT);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->CM_closed), item->flags & AF_CLOSED);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_initial), item->initial);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_minimum), item->minimum);

		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque1), item->cheque1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_cheque2), item->cheque2);

	}

}

/*
static gboolean ui_acc_manage_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
	ui_acc_manage_get(widget, user_data);
	return FALSE;
}
*/

/*
** update the widgets status and contents from action/selection value
*/
static void ui_acc_manage_update(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
GtkTreeModel		 *model;
GtkTreeIter			 iter;
gboolean selected, sensitive;
guint32 key;
//todo: for stock account
//gboolean is_new;

	DB( g_print("\n(ui_acc_manage_update)\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	//window = gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW);
	//DB( g_print("(defpayee) widget=%08lx, window=%08lx, inst_data=%08lx\n", treeview, window, data) );

	//if true there is a selected node
	selected = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_acc)), &model, &iter);
	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));

	DB( g_print(" -> selected = %d  action = %d key = %d\n", selected, data->action, key) );

	//todo amiga/linux
	/*
	if(acc)
	{
	// check for archives related
		for(i=0;;i++)
		{
		struct Archive *arc;

			DoMethod(data->mwd->LV_arc, MUIM_List_GetEntry, i, &arc);
			if(!arc) break;
			if(arc->arc_Account == acc->acc_Id)
			{ nbarc++; break; }
		}

	// check for transaction related
		for(i=0;;i++)
		{
		struct Transaction *ope;

			DoMethod(data->mwd->LV_ope, MUIM_List_GetEntry, i, &ope);
			if(!ope) break;
			if(ope->ope_Account == acc->acc_Id)
			{ nbope++; break; }
		}
	}	*/

	//todo: for stock account
	//todo: lock type if oldpos!=0
/*
	if( selected )
	{
		gtk_tree_model_get(model, &iter,
			LST_DEFACC_NEW, &is_new,
			-1);
		gtk_widget_set_sensitive(data->CY_type, is_new);
	}
*/

	sensitive = (selected == TRUE) ? TRUE : FALSE;
	gtk_widget_set_sensitive(data->CY_type, sensitive);
	//gtk_widget_set_sensitive(data->CY_curr, sensitive);
	gtk_widget_set_sensitive(data->ST_number, sensitive);
	gtk_widget_set_sensitive(data->ST_bank, sensitive);
	gtk_widget_set_sensitive(data->CM_nobudget, sensitive);
	gtk_widget_set_sensitive(data->CM_nosummary, sensitive);
	gtk_widget_set_sensitive(data->CM_noreport, sensitive);
	gtk_widget_set_sensitive(data->CM_closed, sensitive);

	gtk_widget_set_sensitive(data->ST_initial, sensitive);
	gtk_widget_set_sensitive(data->ST_minimum, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque1, sensitive);
	gtk_widget_set_sensitive(data->ST_cheque2, sensitive);


	//sensitive = (data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->LV_acc, sensitive);
	//gtk_widget_set_sensitive(data->BT_new, sensitive);

	sensitive = (selected == TRUE && data->action == 0) ? TRUE : FALSE;
	//gtk_widget_set_sensitive(data->BT_mod, sensitive);
	gtk_widget_set_sensitive(data->BT_rem, sensitive);

	if(selected)
	{
		if(key != data->lastkey)
		{
			DB( g_print(" -> should first do a get for account %d\n", data->lastkey) );
			ui_acc_manage_getlast(data);
		}

		ui_acc_manage_set(widget, NULL);
	}

	data->lastkey = key;

}


/*
** add an empty new account to our temp GList and treeview
*/
static void ui_acc_manage_add(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
Account *item;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(ui_acc_manage_add) (data=%x)\n", (guint)data) );

	gchar *name = dialog_get_name(_("Account name"), NULL, GTK_WINDOW(data->window));
	if(name != NULL)
	{
		if(account_exists(name))
		{
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
				_("Error"),
				_("Cannot add an account '%s',\n"
				"this name already exists."),
				name
				);
		}
		else
		{
			item = da_acc_malloc();
			item->name = name; //g_strdup_printf( _("(account %d)"), da_acc_length()+1);
			//item->kcur = GLOBALS->kcur;

			da_acc_append(item);
			ui_acc_listview_add(GTK_TREE_VIEW(data->LV_acc), item);

			data->change++;
		}
	}
}

/*
** remove the selected account to our treeview and temp GList
*/
static void ui_acc_manage_remove(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
//Account *item;
guint32 key;
gboolean do_remove;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(ui_acc_manage_remove) (data=%x)\n", (guint)data) );

	do_remove = TRUE;
	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
		if( account_is_used(key) == TRUE )
		{		
			//item = da_acc_get(key);
			do_remove = FALSE;
			ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_INFO,
				_("Remove not allowed"),
				_("This account is used and cannot be removed.")
			);
			
		}

		if( do_remove )
		{
			da_acc_remove(key);
			ui_acc_listview_remove_selected(GTK_TREE_VIEW(data->LV_acc));
			data->change++;
		}
	}
}






/*
** rename the selected account to our treeview and temp GList
*/
static void ui_acc_manage_rename(GtkWidget *widget, gpointer user_data)
{
struct ui_acc_manage_data *data;
Account *item;
guint32 key;
gboolean bool;

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW)), "inst_data");
	DB( g_print("\n(ui_acc_manage_rename) (data=%x)\n", (guint)data) );

	key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
	if( key > 0 )
	{
		item = da_acc_get(key);

		gchar *name = dialog_get_name(_("Account name"), item->name, GTK_WINDOW(data->window));
		if(name != NULL)
		{
			if(account_exists(name))
			{
				ui_dialog_msg_infoerror(GTK_WINDOW(data->window), GTK_MESSAGE_ERROR,
					_("Error"),
					_("Cannot rename this Account,\n"
					"from '%s' to '%s',\n"
					"this name already exists."),
					item->name,
					name
				    );
			}
			else
			{
				bool = account_rename(item, name);					
				if(bool)
				{
					gtk_tree_view_columns_autosize (GTK_TREE_VIEW(data->LV_acc));
					data->change++;
				}
			}

		}
		
	}
}


static void ui_acc_manage_rowactivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
//struct account_data *data;

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(treeview), GTK_TYPE_WINDOW)), "inst_data");

	ui_acc_manage_rename(GTK_WIDGET(treeview), NULL);
	
}




/*
**
*/
static void ui_acc_manage_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	ui_acc_manage_update(GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), NULL);
}

//gint ui_acc_manage_list_sort(struct _Account *a, struct _Account *b) { return( a->acc_Id - b->acc_Id); }

/*
**
*/
static gboolean ui_acc_manage_cleanup(struct ui_acc_manage_data *data, gint result)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
guint32 i;
guint32 key;
gboolean doupdate = FALSE;

	DB( g_print("\n(ui_acc_manage_cleanup) %x\n", (guint)data) );

		key = ui_acc_listview_get_selected_key(GTK_TREE_VIEW(data->LV_acc));
		if(key > 0)
		{
			data->lastkey = key;
			DB( g_print(" -> should first do a get for account %d\n", data->lastkey) );
			ui_acc_manage_getlast(data);
		}

		// test for change & store new position
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_acc));
		i=1; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
		while (valid)
		{
		Account *item;

			gtk_tree_model_get(GTK_TREE_MODEL(model), &iter,
				LST_DEFACC_DATAS, &item,
				-1);

			DB( g_print(" -> check acc %d, pos is %d, %s\n", i, item->pos, item->name) );

			if(item->pos != i)
				data->change++;

			item->pos = i;

			// Make iter point to the next row in the list store 
			i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
		}

	GLOBALS->changes_count += data->change;

	return doupdate;
}

/*
**
*/
static void ui_acc_manage_setup(struct ui_acc_manage_data *data)
{

	DB( g_print("\n(ui_acc_manage_setup)\n") );

	//init GList
	data->tmp_list = NULL; //hb-glist_clone_list(GLOBALS->acc_list, sizeof(struct _Account));
	data->action = 0;
	data->change = 0;
	data->lastkey = 0;

	ui_acc_listview_populate(data->LV_acc, ACC_LST_INSERT_NORMAL);
	//ui_cur_combobox_populate(data->CY_curr, GLOBALS->h_cur);
	//populate_view_acc(data->LV_acc, GLOBALS->acc_list, TRUE);
}

/*
**
*/
GtkWidget *ui_acc_manage_dialog (void)
{
struct ui_acc_manage_data data;
GtkWidget *dialog, *content, *mainbox, *scrollwin, *notebook, *alignment;
GtkWidget *table, *label, *widget, *hpaned;
gint row;

	dialog = gtk_dialog_new_with_buttons (_("Manage Accounts"),
				GTK_WINDOW(GLOBALS->mainwindow),
				0,
			    GTK_STOCK_CLOSE,
			    GTK_RESPONSE_ACCEPT,
				NULL);

	data.window = dialog;

	//set the dialog icon
	//homebank_window_set_icon_from_file(GTK_WINDOW (dialog), "account.svg");	
	gtk_window_set_icon_name(GTK_WINDOW (dialog), HB_STOCK_ACCOUNT);

	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)&data);
	DB( g_print("(ui_acc_manage_) dialog=%x, inst_data=%x\n", (guint)dialog, (guint)&data) );

	//window contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
	mainbox = gtk_hbox_new (FALSE, HB_BOX_SPACING);
	gtk_box_pack_start (GTK_BOX (content), mainbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(mainbox), HB_MAINBOX_SPACING);

	hpaned = gtk_hpaned_new();
	gtk_box_pack_start (GTK_BOX (mainbox), hpaned, TRUE, TRUE, 0);

	/* left area */
	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_paned_pack1 (GTK_PANED(hpaned), table, FALSE, FALSE);

	row = 0;
 	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	data.LV_acc = ui_acc_listview_new(FALSE);
	gtk_widget_set_size_request(data.LV_acc, HB_MINWIDTH_LIST, -1);
	gtk_container_add(GTK_CONTAINER(scrollwin), data.LV_acc);
	gtk_widget_set_tooltip_text(data.LV_acc, _("Drag & drop to change the order\nDouble-click to rename"));
	gtk_table_attach_defaults (GTK_TABLE (table), scrollwin, 0, 2, row, row+1);

	// tools buttons
	row++;
	widget = gtk_button_new_from_stock(GTK_STOCK_ADD);
	data.BT_add = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 0, 1, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);
	widget = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	data.BT_rem = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), (GtkAttachOptions) (0), 0, 0);


	/* right area */
	notebook = gtk_notebook_new();
	//gtk_box_pack_start (GTK_BOX (mainbox), notebook, TRUE, TRUE, 0);
	gtk_paned_pack2 (GTK_PANED(hpaned), notebook, FALSE, FALSE);
	
	/* page general */
	table = gtk_table_new (8, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(table), HB_MAINBOX_SPACING);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	label = gtk_label_new(_("General"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), alignment, label);

	row = 0;
	label = make_label(_("Account"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Type:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_cycle(label, CYA_ACC_TYPE);
	data.CY_type = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	/*
	row++;
	label = make_label(_("_Currency:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = ui_cur_combobox_new(label);
	data.CY_curr = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	*/

	row++;
	label = make_label (_("Start _balance:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	widget = make_amount(label);
	data.ST_initial = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	//TODO: notes

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("this account was _closed"));
	data.CM_closed = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	
	row++;
	label = make_label(_("Current check number"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("Checkbook _1:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_long (label);
	data.ST_cheque1 = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Checkbook _2:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_long (label);
	data.ST_cheque2 = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	/* page options */
	table = gtk_table_new (9, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), HB_TABROW_SPACING);
	gtk_table_set_col_spacings (GTK_TABLE (table), HB_TABCOL_SPACING);
	gtk_container_set_border_width (GTK_CONTAINER(table), HB_MAINBOX_SPACING);
	//			gtk_alignment_new(xalign, yalign, xscale, yscale)
	alignment = gtk_alignment_new(0.5, 0, 1.0, 0.0);
	gtk_container_add(GTK_CONTAINER(alignment), table);
	label = gtk_label_new(_("Options"));
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), alignment, label);

	
	row = 0;
	label = make_label(_("Institution"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	label = make_label(_("_Name:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1,(GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data.ST_bank = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("N_umber:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_string(label);
	data.ST_number = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Limits"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	//TODO: warning/absolute minimum balance

	row++;
	label = make_label (_("_Min. balance:"), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 1, 2, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
	widget = make_amount(label);
	data.ST_minimum = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 2, 3, row, row+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	label = make_label(_("Report exclusion"), 0.0, 0.5);
	gimp_label_set_attributes(GTK_LABEL(label), PANGO_ATTR_WEIGHT, PANGO_WEIGHT_BOLD, -1);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 3, row, row+1);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("exclude from account _summary"));
	data.CM_nosummary = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("exclude from the _budget"));
	data.CM_nobudget = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);

	row++;
	widget = gtk_check_button_new_with_mnemonic (_("exclude from any _reports"));
	data.CM_noreport = widget;
	gtk_table_attach (GTK_TABLE (table), widget, 1, 3, row, row+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);


	//connect all our signals
	g_signal_connect (dialog, "destroy", G_CALLBACK (gtk_widget_destroyed), &dialog);
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data.LV_acc)), "changed", G_CALLBACK (ui_acc_manage_selection), NULL);
	g_signal_connect (GTK_TREE_VIEW(data.LV_acc), "row-activated", G_CALLBACK (ui_acc_manage_rowactivated), GINT_TO_POINTER(2));
	
	g_signal_connect (G_OBJECT (data.BT_add), "clicked", G_CALLBACK (ui_acc_manage_add), NULL);
	g_signal_connect (G_OBJECT (data.BT_rem), "clicked", G_CALLBACK (ui_acc_manage_remove), NULL);

	//setup, init and show window
	ui_acc_manage_setup(&data);
	ui_acc_manage_update(data.LV_acc, NULL);

//	gtk_window_set_default_size (GTK_WINDOW (window), 640, 480);

	gtk_widget_show_all (dialog);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	// cleanup and destroy
	ui_acc_manage_cleanup(&data, result);
	gtk_widget_destroy (dialog);

	return NULL;
}


