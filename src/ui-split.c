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

#include "ui-split.h"
#include "ui-transaction.h"
#include "ui-archive.h"
#include "gtk-dateentry.h"
#include "ui-payee.h"
#include "ui-category.h"
#include "ui-account.h"
#include "hb-split.h"




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


#define GTK_RESPONSE_SPLIT_REM 10888


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void list_split_number_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
GtkTreePath *path;
gint *indices;
gchar num[16];

	path = gtk_tree_model_get_path(model, iter);
	indices = gtk_tree_path_get_indices(path);
	//num = gtk_tree_path_to_string(path);
	g_snprintf(num, 15, "%d", 1 + *indices);
	gtk_tree_path_free(path);

    g_object_set(renderer, "text", num, NULL);

}


static void list_split_amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Split *split;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble amount;
gchar *color;

	gtk_tree_model_get(model, iter, 0, &split, -1);

	//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, ope->kcur, GLOBALS->minor);
	amount = split->amount;
	g_snprintf(buf, G_ASCII_DTOSTR_BUF_SIZE-1, "%.2f", amount);

	color = get_normal_color_amount(amount);
	g_object_set(renderer,
		"foreground",  color,
		"text", buf,
		NULL);
	
}


static void list_split_memo_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Split *split;

	gtk_tree_model_get(model, iter, 0, &split, -1);
	
    g_object_set(renderer, "text", split->memo, NULL);
}


static void list_split_category_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Split *split;
Category *cat;

	gtk_tree_model_get(model, iter, 0, &split, -1);

	cat = da_cat_get(split->kcat);
	if( cat != NULL )
	{
		g_object_set(renderer, "text", cat->fullname, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);
}


static void list_split_populate(struct ui_split_dialog_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
Split *split;
gint count, i;

	DB( g_print("\n[list_split] populate\n") );

	count = da_splits_length (data->tmp_splits);

	if( count <= 0 )
		return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split));

	gtk_list_store_clear (GTK_LIST_STORE(model));

	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_split), NULL); /* Detach model from view */

	/* populate */
	for(i=0 ; i < count ; i++)
	{
		split = da_splits_get(data->tmp_splits, i);

		DB( g_print("- set split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo) );

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, split,
			-1);

	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(data->LV_split), model); /* Re-attach model to view */
	g_object_unref(model);
}





static GtkWidget *
list_split_new(void)
{
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer		*renderer;
GtkTreeViewColumn	*column;

	DB( g_print("\n[ui_split_listview] new\n") );


	// create list store
	store = gtk_list_store_new(1,
		G_TYPE_POINTER
		);

	// treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);

	//column 0: line number
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes("#", renderer, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_split_number_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	// column 1: category
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);
	
	column = gtk_tree_view_column_new_with_attributes(_("Category"), renderer, NULL);

	//gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);

	//gtk_tree_view_column_set_sort_column_id (column, sortcolumnid);
	//gtk_tree_view_column_set_fixed_width( column, HB_MINWIDTH_LIST);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_split_category_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	
	// column 2: memo
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);
	
	column = gtk_tree_view_column_new_with_attributes(_("Memo"), renderer, NULL);

	//gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);

	//gtk_tree_view_column_set_sort_column_id (column, sortcolumnid);
	//gtk_tree_view_column_set_fixed_width( column, HB_MINWIDTH_LIST);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_split_memo_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	// column 3: amount
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);

	column = gtk_tree_view_column_new_with_attributes(_("Amount"), renderer, NULL);
	
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_set_sort_column_id (column, sortcolumnid);
	gtk_tree_view_column_set_fixed_width( column, HB_MINWIDTH_LIST);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_split_amount_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	// column empty
	//column = gtk_tree_view_column_new();
	//gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);


	// treeviewattribute
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_reorderable (GTK_TREE_VIEW(treeview), TRUE);
	
	//gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(store), ui_acc_listview_compare_func, NULL, NULL);
	//gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	return treeview;
}




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static void ui_split_dialog_filter_text_handler (GtkEntry    *entry,
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
    if (text[i]=='|')
      continue;
    result[count++] = text[i];
  }


  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (ui_split_dialog_filter_text_handler),
                                     data);
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (ui_split_dialog_filter_text_handler),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert-text");

  g_free (result);
}


static void ui_split_dialog_cb_eval_order(struct ui_split_dialog_data *data)
{
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;
guint i;

	DB( g_print("\n[ui_split_dialog] eval order\n") );

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split));
	i=1; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Split *split;

		gtk_tree_model_get (model, &iter, 0, &split, -1);
		split->pos = i;

		DB( g_print("split pos: %d '%s' %.2f\n", i, split->memo, split->amount) );
		
		i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}

	da_splits_sort(data->tmp_splits);
}


static void ui_split_dialog_cb_eval_split(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
gboolean tmpval = FALSE;
gdouble amount;
gint count;
	
	DB( g_print("\n[ui_split_dialog] eval split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	amount= g_strtod(gtk_entry_get_text(GTK_ENTRY(data->ST_amount)), NULL);

	tmpval = hb_amount_round(amount, 2) != 0.0 ? TRUE : FALSE;
	gtk_widget_set_sensitive (data->BT_apply, tmpval);

	count = gtk_tree_model_iter_n_children(gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split)), NULL);
	if( count >= TXN_MAX_SPLIT )
		tmpval = FALSE;
	gtk_widget_set_sensitive (data->BT_add, tmpval);

	DB( g_print(" - txt='%s' amt=%.2f, nbsplit=%d, valid=%d\n", gtk_entry_get_text(GTK_ENTRY(data->ST_amount)), amount, count, tmpval) );

}


static void ui_split_dialog_update(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
gboolean tmpval;
guint count;

	DB( g_print("\n[ui_split_dialog] update\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	count = da_splits_length (data->tmp_splits);

	//btn: edit/rem
	tmpval = gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_split)), NULL, NULL);
	gtk_widget_set_sensitive (data->BT_edit, (data->isedited) ? FALSE : tmpval);
	gtk_widget_set_sensitive (data->BT_rem, (data->isedited) ? FALSE : tmpval);

	//btn: remall
	tmpval = (count > 1) ? TRUE : FALSE;
	gtk_widget_set_sensitive (data->BT_remall, (data->isedited) ? FALSE : tmpval);

	ui_split_dialog_cb_eval_split(widget, NULL);
	
	//btn: add/apply
	/*amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
	tmpval = hb_amount_round(amount, 2) != 0.0 ? TRUE : FALSE;
	gtk_widget_set_sensitive (data->BT_apply, tmpval);

	if( count >= TXN_MAX_SPLIT )
		tmpval = FALSE;
	gtk_widget_set_sensitive (data->BT_add, tmpval);
	*/
	
	//btn: show/hide
	gtk_widget_set_sensitive (data->LV_split, !data->isedited);

	hb_widget_visible (data->BT_add, !data->isedited);
	
	hb_widget_visible (data->IM_edit, data->isedited);
	hb_widget_visible (data->BT_apply, data->isedited);
	hb_widget_visible (data->BT_cancel, data->isedited);
}


static void ui_split_dialog_edit_end(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;

	DB( g_print("\n[ui_split_dialog] edit_end\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	g_signal_handler_block(data->PO_cat, data->hid_cat);
	g_signal_handler_block(data->ST_amount, data->hid_amt);

	ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), 0.0);
	gtk_entry_set_text(GTK_ENTRY(data->ST_memo), "");

	g_signal_handler_unblock(data->ST_amount, data->hid_amt);
	g_signal_handler_unblock(data->PO_cat, data->hid_cat);

	data->isedited = FALSE;
}


static void ui_split_dialog_edit_start(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("\n[ui_split_dialog] edit_start\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_split));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Split *split;
	gchar *txt;

		gtk_tree_model_get(model, &iter, 0, &split, -1);

		ui_cat_comboboxentry_set_active(GTK_COMBO_BOX(data->PO_cat), split->kcat);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->ST_amount), split->amount);
		txt = (split->memo != NULL) ? split->memo : "";
		gtk_entry_set_text(GTK_ENTRY(data->ST_memo), txt);
		
		data->isedited = TRUE;

		ui_split_dialog_update (data->dialog, user_data);
	}
}



static void ui_split_dialog_cancel_cb(GtkWidget *widget, gpointer user_data)
{
//struct ui_split_dialog_data *data;

	DB( g_print("\n[ui_split_dialog] cancel\n") );

	//data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	ui_split_dialog_edit_end(widget, user_data);
	ui_split_dialog_update (widget, user_data);
}


static void ui_split_dialog_apply_cb(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("\n[ui_split_dialog] apply\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_split));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Split *split;
	gdouble amount;

		gtk_tree_model_get(model, &iter, 0, &split, -1);
		gtk_spin_button_update (GTK_SPIN_BUTTON(data->ST_amount));
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if(amount)
		{
			split->kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_cat));
			g_free(split->memo);
			split->memo = g_strdup((gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_memo)));
			split->amount = amount;
		}
	}

	ui_split_dialog_edit_end(widget, user_data);
	ui_split_dialog_compute (widget, data);
	ui_split_dialog_update (widget, user_data);
}


static void ui_split_dialog_removeall_cb(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;

	DB( g_print("\n[ui_split_dialog] removeall_cb\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(data->LV_split))));
	da_split_destroy(data->tmp_splits);
	data->tmp_splits = da_split_new ();
	
	ui_split_dialog_compute (widget, data);
	ui_split_dialog_update (widget, user_data);
}


static void ui_split_dialog_remove_cb(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
GtkTreeSelection *selection;
GtkTreeModel		 *model;
GtkTreeIter			 iter;

	DB( g_print("\n[ui_split_dialog] remove_cb\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_split));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
	Split *split;

		gtk_tree_model_get(model, &iter, 0, &split, -1);
		//todo: not implemented yet
		da_splits_remove(data->tmp_splits, split);
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	}
	
	ui_split_dialog_compute (widget, data);
	ui_split_dialog_update (widget, user_data);
}


static void ui_split_dialog_add_cb(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
GtkTreeModel *model;
GtkTreeIter	iter;
Split *split;
guint count;
gdouble amount;

	DB( g_print("\n[ui_split_dialog] add\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	count = da_splits_length (data->tmp_splits);
	if( count <= TXN_MAX_SPLIT )
	{
		split = da_split_malloc ();
		gtk_spin_button_update (GTK_SPIN_BUTTON(data->ST_amount));
		amount = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->ST_amount));
		if(amount)
		{
			split->amount = amount;
			split->kcat = ui_cat_comboboxentry_get_key_add_new(GTK_COMBO_BOX(data->PO_cat));
			split->memo = g_strdup((gchar *)gtk_entry_get_text(GTK_ENTRY(data->ST_memo)));

			DB( g_print("- get split : %d, %.2f, %s\n", split->kcat, split->amount, split->memo) );

			da_splits_append (data->tmp_splits, split);

			model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split));
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, split,
				-1);

			ui_split_dialog_compute (widget, data);
			
		}
		else
		{
			//todo: msg max number reached
			da_split_free(split);
		}
	}
	
	ui_split_dialog_edit_end(widget, user_data);
	ui_split_dialog_update (widget, user_data);
}


static void ui_split_dialog_cb_activate_split(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;

	DB( g_print("\n[ui_split_dialog] cb activate split\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	if( data->isedited == TRUE )
		ui_split_dialog_apply_cb(widget, NULL);
	else
		ui_split_dialog_add_cb(widget, NULL);
}


static void ui_split_rowactivated (GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer user_data)
{
	DB( g_print("\n[ui_split_dialog] rowactivated\n") );

	ui_split_dialog_edit_start(GTK_WIDGET(treeview), NULL);
}


static void ui_split_selection(GtkTreeSelection *treeselection, gpointer user_data)
{
	DB( g_print("\n[ui_split_dialog] selection\n") );

	ui_split_dialog_update (GTK_WIDGET(gtk_tree_selection_get_tree_view (treeselection)), user_data);
}


void ui_split_dialog_compute(GtkWidget *widget, gpointer user_data)
{
struct ui_split_dialog_data *data;
gint i, count, nbvalid;
gchar buf[48];
gboolean sensitive;
GtkTreeModel *model;
GtkTreeIter	iter;
gboolean valid;

	DB( g_print("\n[ui_split_dialog] compute\n") );

	data = g_object_get_data(G_OBJECT(gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW)), "inst_data");

	nbvalid = 0;
	data->sumsplit = 0.0;
	data->remsplit = 0.0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(data->LV_split));
	i=0; valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
	while (valid)
	{
	Split *split;

		gtk_tree_model_get (model, &iter,
			0, &split,
			-1);

		data->sumsplit += split->amount;
		if( hb_amount_round(split->amount, 2) != 0.0 )
			 nbvalid++;

		/* Make iter point to the next row in the list store */
		i++; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
	}
	count = i;

	DB( g_print("- count=%d, nbvalid=%d\n", count, nbvalid ) );
	
	data->remsplit = data->amount - data->sumsplit;


	//validation: 2 split min
	sensitive = FALSE;
	if( (count == 0) || nbvalid >= 2 )
		sensitive = TRUE;
	gtk_dialog_set_response_sensitive(GTK_DIALOG(data->dialog), GTK_RESPONSE_ACCEPT, sensitive);

	if( hb_amount_round(data->amount, 2) != 0.0 )
	{
		if(!data->remsplit)
			g_sprintf(buf, "----");
		else
			g_snprintf(buf, 48, "%.2f", data->remsplit);

		gtk_label_set_label(GTK_LABEL(data->LB_remain), buf);

		g_snprintf(buf, 48, "%.2f", data->amount);
		gtk_label_set_label(GTK_LABEL(data->LB_txnamount), buf);
	}

	g_snprintf(buf, 48, "%.2f", data->sumsplit);
	gtk_label_set_text(GTK_LABEL(data->LB_sumsplit), buf);
	
}


static void ui_split_dialog_setup(struct ui_split_dialog_data *data)
{
guint count;

	DB( g_print("\n[ui_split_dialog] set\n") );

	count = da_splits_length(data->tmp_splits);
	data->nbsplit = count > 1 ? count-1 : 0;

	DB( g_print("- count = %d\n", count) );
	list_split_populate (data);

	data->isedited = FALSE;

	ui_cat_comboboxentry_populate(GTK_COMBO_BOX(data->PO_cat), GLOBALS->h_cat);

	ui_split_dialog_compute(data->dialog, data);
	ui_split_dialog_update (data->dialog, data);
}


GtkWidget *ui_split_dialog (GtkWidget *parent, GPtrArray **src_splits, gdouble amount, void (update_callbackFunction(GtkWidget*, gdouble)))
{
struct ui_split_dialog_data *data;
GtkWidget *dialog, *content, *table, *box, *scrollwin;
GtkWidget *label, *widget;
gint row;

	DB( g_print("\n[ui_split_dialog] new\n") );

	data = g_malloc0(sizeof(struct ui_split_dialog_data));

	dialog = gtk_dialog_new_with_buttons (_("Transaction splits"),
					    GTK_WINDOW(parent),
					    0,
					    _("_Cancel"),
					    GTK_RESPONSE_CANCEL,
					    NULL);

	//store our dialog private data
	g_object_set_data(G_OBJECT(dialog), "inst_data", (gpointer)data);
	DB( g_print(" - window=%p, inst_data=%p\n", dialog, data) );

    g_signal_connect (dialog, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &dialog);

	data->dialog = dialog;

	//gtk_dialog_add_button(GTK_DIALOG(dialog), _("_Remove"), GTK_RESPONSE_SPLIT_REM);
	gtk_dialog_add_button(GTK_DIALOG(dialog), _("_OK"), GTK_RESPONSE_ACCEPT);
	
	//todo: init should move 
	//clone splits or create new
	data->src_splits = *src_splits;
	data->amount = amount;
	data->sumsplit = amount;
	
	if( *src_splits != NULL )
		data->tmp_splits = da_splits_clone(*src_splits); 
	else
		data->tmp_splits = da_split_new();

	//dialog contents
	content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));

	table = gtk_grid_new ();
	gtk_container_set_border_width (GTK_CONTAINER (table), SPACING_SMALL);
	gtk_grid_set_row_spacing (GTK_GRID (table), SPACING_TINY);
	gtk_grid_set_column_spacing (GTK_GRID (table), SPACING_TINY);
	gtk_box_pack_start (GTK_BOX (content), table, TRUE, TRUE, 0);

	row = 0;

	scrollwin = gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrollwin), GTK_SHADOW_ETCHED_IN);
	gtk_widget_set_size_request(scrollwin, HB_MINWIDTH_LIST, HB_MINHEIGHT_LIST);
	gtk_widget_set_hexpand (scrollwin, TRUE);
	gtk_widget_set_vexpand (scrollwin, TRUE);
	data->LV_split = list_split_new();
	gtk_container_add(GTK_CONTAINER(scrollwin), data->LV_split);
	gtk_grid_attach (GTK_GRID (table), scrollwin, 0, row, 4, 1);

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, SPACING_TINY);
	gtk_widget_set_valign (box, GTK_ALIGN_CENTER);
	gtk_grid_attach (GTK_GRID (table), box, 4, row, 1, 1);

		widget = make_image_button(ICONNAME_LIST_REMOVE_ALL, _("Remove all"));
		data->BT_remall = widget;
		gtk_box_pack_end (GTK_BOX (box), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_LIST_REMOVE, _("Remove"));
		data->BT_rem = widget;
		gtk_box_pack_end (GTK_BOX(box), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_HB_OPE_EDIT, _("Edit"));
		data->BT_edit = widget;
		gtk_box_pack_end (GTK_BOX(box), widget, FALSE, FALSE, 0);

	row++;
	label = gtk_label_new(_("Category"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 0, row, 1, 1);

	label = gtk_label_new(_("Memo"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);

	label = gtk_label_new(_("Amount"));
	gimp_label_set_attributes (GTK_LABEL (label), PANGO_ATTR_SCALE, PANGO_SCALE_SMALL, -1);
	gtk_grid_attach (GTK_GRID (table), label, 2, row, 1, 1);


	row++;
	widget = ui_cat_comboboxentry_new(NULL);
	data->PO_cat = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 0, row, 1, 1);

	widget = make_string(NULL);
	data->ST_memo= widget;
	gtk_grid_attach (GTK_GRID (table), widget, 1, row, 1, 1);

	widget = make_amount(NULL);
	data->ST_amount = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SPACING_TINY);
	gtk_grid_attach (GTK_GRID (table), box, 3, row, 1, 1);

		widget = gtk_image_new_from_icon_name (ICONNAME_HB_OPE_EDIT, GTK_ICON_SIZE_BUTTON);
		data->IM_edit = widget;
		gtk_box_pack_start (GTK_BOX(box), widget, TRUE, TRUE, 0);

		widget = make_image_button(ICONNAME_LIST_ADD, _("Add"));
		data->BT_add = widget;
		gtk_box_pack_start (GTK_BOX(box), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_EMBLEM_OK, _("Apply"));
		data->BT_apply = widget;
		gtk_box_pack_start (GTK_BOX(box), widget, FALSE, FALSE, 0);

		widget = make_image_button(ICONNAME_WINDOW_CLOSE, _("Cancel"));
		data->BT_cancel = widget;
		gtk_box_pack_start (GTK_BOX(box), widget, FALSE, FALSE, 0);


	if( hb_amount_round(data->amount, 2) != 0.0 )
	{
		row++;
		label = gtk_label_new(_("Transaction amount:"));
		gtk_widget_set_halign (label, GTK_ALIGN_END);
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		widget = gtk_label_new(NULL);
		gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
		data->LB_txnamount = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

		row++;
		label = gtk_label_new(_("Unassigned:"));
		gtk_widget_set_halign (label, GTK_ALIGN_END);
		gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
		widget = gtk_label_new(NULL);
		gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
		data->LB_remain = widget;
		gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);

		row++;
		widget = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
		gtk_grid_attach (GTK_GRID (table), widget, 1, row, 2, 1);

	}

	row++;
	label = gtk_label_new(_("Sum of splits:"));
	gtk_widget_set_halign (label, GTK_ALIGN_END);
	gtk_grid_attach (GTK_GRID (table), label, 1, row, 1, 1);
	widget = gtk_label_new(NULL);
	gtk_widget_set_halign (widget, GTK_ALIGN_CENTER);
	data->LB_sumsplit = widget;
	gtk_grid_attach (GTK_GRID (table), widget, 2, row, 1, 1);


	//connect all our signals
	g_signal_connect (gtk_tree_view_get_selection(GTK_TREE_VIEW(data->LV_split)), "changed", G_CALLBACK (ui_split_selection), data);
	g_signal_connect (GTK_TREE_VIEW(data->LV_split), "row-activated", G_CALLBACK (ui_split_rowactivated), data);

	g_signal_connect (data->ST_memo, "insert-text", G_CALLBACK(ui_split_dialog_filter_text_handler), data);
	
	data->hid_cat = g_signal_connect (data->PO_cat , "changed"    , G_CALLBACK (ui_split_dialog_cb_eval_split), data);
	data->hid_amt = g_signal_connect (data->ST_amount, "changed", G_CALLBACK (ui_split_dialog_cb_eval_split), data);
	//data->hid_amt = g_signal_connect (data->ST_amount, "value-changed", G_CALLBACK (ui_split_dialog_cb_eval_split), data);

	//todo: add enter validate
	g_signal_connect (data->ST_amount, "activate", G_CALLBACK (ui_split_dialog_cb_activate_split), NULL);

	
	g_signal_connect (data->BT_edit  , "clicked", G_CALLBACK (ui_split_dialog_edit_start), NULL);
	g_signal_connect (data->BT_rem   , "clicked", G_CALLBACK (ui_split_dialog_remove_cb), NULL);
	g_signal_connect (data->BT_remall, "clicked", G_CALLBACK (ui_split_dialog_removeall_cb), NULL);

	g_signal_connect (data->BT_add   , "clicked", G_CALLBACK (ui_split_dialog_add_cb), NULL);
	g_signal_connect (data->BT_apply , "clicked", G_CALLBACK (ui_split_dialog_apply_cb), NULL);
	g_signal_connect (data->BT_cancel, "clicked", G_CALLBACK (ui_split_dialog_cancel_cb), NULL);

	//gtk_window_set_default_size(GTK_WINDOW(dialog), 480, -1);
	gtk_widget_show_all (dialog);

	//setup, init and show dialog
	ui_split_dialog_setup(data);

	//wait for the user
	gint result = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (result)
    {
	// sum split and alter txn amount   	
	case GTK_RESPONSE_ACCEPT:
		if( da_splits_length(data->tmp_splits) )
		{
			ui_split_dialog_cb_eval_order(data);
			// here we swap src_splits <> tmp_splits
			*src_splits = data->tmp_splits;
			data->tmp_splits = data->src_splits;
			update_callbackFunction(parent, data->sumsplit);
		}
		else
		{
			//remove split and revert back original amount
			da_split_destroy(*src_splits);
			*src_splits = NULL;
			update_callbackFunction(parent, data->amount);
		}
		break;
	/*case GTK_RESPONSE_SPLIT_REM:
		da_split_destroy(*src_splits);
		*src_splits = NULL;
		update_callbackFunction(parent, data->sumsplit);
		break;
		*/
	default:
	   //do_nothing_since_dialog_was_cancelled ();
	   break;
    }

	// debug
	/*#if MYDEBUG == 1
	{
	guint i;

		for(i=0;i<TXN_MAX_SPLIT;i++)
		{
		Split *split = data->ope_splits[i];
			if(data->ope_splits[i] == NULL)
				break;
			g_print(" split %d : %d, %.2f, %s\n", i, split->kcat, split->amount, split->memo);
		}
	}
	#endif*/

	// cleanup and destroy
	//GLOBALS->changes_count += data->change;
	gtk_widget_destroy (dialog);
	
	da_split_destroy (data->tmp_splits);
	g_free(data);

	return NULL;
}

