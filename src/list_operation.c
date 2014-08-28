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

#include "list_operation.h"

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


//debug
//extern gboolean minor_active;
extern GdkPixbuf *paymode_icons[];

/* This is not pretty. Of course you can also use a
   *  separate compare function for each sort ID value */

static gint ope_sort_iter_compare_strings(gchar *s1, gchar *s2)
{
	return hb_string_utf8_compare(s1, s2);
}


static   gint
  ope_sort_iter_compare_func (GtkTreeModel *model,
                          GtkTreeIter  *a,
                          GtkTreeIter  *b,
                          gpointer      userdata)
  {
    gint sortcol = GPOINTER_TO_INT(userdata);
    gint retval = 0;
	Transaction *ope1, *ope2;
	gdouble tmpval = 0;

	gtk_tree_model_get(model, a, LST_DSPOPE_DATAS, &ope1, -1);
	gtk_tree_model_get(model, b, LST_DSPOPE_DATAS, &ope2, -1);

    switch (sortcol)
    {
		case LST_DSPOPE_STATUS:
			if(!(retval = (ope1->flags & OF_VALID) - (ope2->flags & OF_VALID) ) )
			{
				retval = (ope1->flags & OF_REMIND) - (ope2->flags & OF_REMIND);
			}
			break;

		case LST_DSPOPE_DATE:
 			if(! (retval = ope1->date - ope2->date) )
			{
				//g_print("sort on balance d1=%d, d2=%d %f %f\n", ope1->date, ope2->date, ope1->balance , ope2->balance);

				tmpval = ope1->pos - ope2->pos;
				retval = tmpval > 0 ? 1 : -1;
			}
			//g_print("ret=%d\n", ret);
			break;

		case LST_DSPOPE_ACCOUNT:
			{
			Account *a1, *a2;

				a1 = da_acc_get(ope1->kacc);
				a2 = da_acc_get(ope2->kacc);
				if( a1 != NULL && a2 != NULL )
				{
					retval = ope_sort_iter_compare_strings(a1->name, a2->name);
				}
			}
			break;

		case LST_DSPOPE_INFO:
			if(!(retval = ope1->paymode - ope2->paymode))
			{
				retval = ope_sort_iter_compare_strings(ope1->info, ope2->info);
			}
			break;

		case LST_DSPOPE_PAYEE:
			{
			Payee *p1, *p2;

				p1 = da_pay_get(ope1->kpay);
				p2 = da_pay_get(ope2->kpay);
				if( p1 != NULL && p2 != NULL )
				{
					retval = ope_sort_iter_compare_strings(p1->name, p2->name);
				}
			}
			break;

		case LST_DSPOPE_WORDING:
				retval = ope_sort_iter_compare_strings(ope1->wording, ope2->wording);
			break;

		case LST_DSPOPE_AMOUNT:
		case LST_DSPOPE_EXPENSE:
		case LST_DSPOPE_INCOME:
			tmpval = ope1->amount - ope2->amount;
			retval = tmpval > 0 ? 1 : -1;
			break;

		case LST_DSPOPE_CATEGORY:
			{
			Category *c1, *c2;
			gchar *name1, *name2;

				c1 = da_cat_get(ope1->kcat);
				c2 = da_cat_get(ope2->kcat);
				if( c1 != NULL && c2 != NULL )
				{
					name1 = da_cat_get_fullname(c1);
					name2 = da_cat_get_fullname(c2);
					retval = ope_sort_iter_compare_strings(name1, name2);
					g_free(name2);
					g_free(name1);
				}
			}
			break;

		case LST_DSPOPE_TAGS:
		{
		gchar *t1, *t2;

			t1 = transaction_tags_tostring(ope1);
			t2 = transaction_tags_tostring(ope2);
			retval = ope_sort_iter_compare_strings(t1, t2);
			g_free(t1);
			g_free(t2);
		}
		break;

		default:
			g_return_val_if_reached(0);
    }

    return retval;
}

/*
** date cell function
*/
static void ope_status_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *entry;
GdkPixbuf *pixbuf = NULL;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &entry, -1);

	/*
		stat[0] = ( entry->ope_Flags & OF_ADDED  ) ? data->istatus[2] : data->istatus[0];
		stat[1] = ( entry->ope_Flags & OF_CHANGED) ? data->istatus[3] : data->istatus[0];
		stat[2] = ( entry->ope_Flags & OF_VALID  ) ? data->istatus[4] : data->istatus[0];
		if( entry->ope_Flags & OF_REMIND ) stat[2] = data->istatus[1];
	*/

	switch(GPOINTER_TO_INT(user_data))
	{
		case 1:
			pixbuf = ( entry->flags & OF_AUTO  ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_AUTO] : ( entry->flags & OF_ADDED ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_ADD] : NULL;
			break;
		case 2:
			pixbuf = ( entry->flags & OF_CHANGED  ) ? GLOBALS->lst_pixbuf[LST_PIXBUF_EDIT] : NULL;
			break;
		case 3:
			if( entry->flags & OF_VALID )
				pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_VALID];
			else
			{
				if( entry->flags & OF_REMIND )
					pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_REMIND];
			}
			break;
	}
	g_object_set(renderer, "pixbuf", pixbuf, NULL);
}


/*
** account cell function
*/
static void ope_account_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
Account *acc;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	acc = da_acc_get(ope->kacc);
	if( acc )
	{
		g_object_set(renderer, "text", acc->name, NULL);
	}
	else
		g_object_set(renderer, "text", "", NULL);
}

/*
** date cell function
*/
static void ope_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar buffer[256];
GDate *date;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	if(ope->date > 0)
	{
		date = g_date_new_julian (ope->date);
		g_date_strftime (buffer, 256-1, PREFS->date_format, date);
		g_date_free(date);

		//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

    	g_object_set(renderer, "text", buffer, NULL);
    }
    else
    	g_object_set(renderer, "text", "????", NULL);
}

/*
** info cell function
*/
static void ope_info_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	switch(GPOINTER_TO_INT(user_data))
	{
		case 1:
			g_object_set(renderer, "pixbuf", paymode_icons[ope->paymode], NULL);
			break;
		case 2:
		    g_object_set(renderer, "text", ope->info, NULL);
			break;
	}
}

/*
** payee cell function
*/
static void ope_payee_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
Payee *pay;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	pay = da_pay_get(ope->kpay);
	if(pay != NULL)
		g_object_set(renderer, "text", pay->name, NULL);
	else
		g_object_set(renderer, "text", "", NULL);
}

/*
** tags cell function
*/
static void ope_tags_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar *str;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	if(ope->tags != NULL)
	{
		str = transaction_tags_tostring(ope);
		g_object_set(renderer, "text", str, NULL);
		g_free(str);
	}
	else
		g_object_set(renderer, "text", "", NULL);


}


/*
** wording cell function
*/
static void ope_wording_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
    g_object_set(renderer, "text", ope->wording, NULL);
}

/*
** amount cell function
*/
static void ope_amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gint column = GPOINTER_TO_INT(user_data);
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gint type;
gdouble amount;
gchar *color;

	// get the transaction
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	if(column == LST_DSPOPE_BALANCE)
		amount = ope->balance;
	else
		amount = ope->amount;


	if(column == LST_DSPOPE_INCOME || column == LST_DSPOPE_EXPENSE)
	{
		type = (ope->flags & OF_INCOME) ? LST_DSPOPE_INCOME : LST_DSPOPE_EXPENSE;
		if( type != column)
		{
			g_object_set(renderer, "markup", NULL, NULL);
			return;
		}
	}

	//if(amount != 0)
	{
		//mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, GLOBALS->minor);

		//todo: optimize this
		//store to a data set to the listview
		//acc = da_acc_get(ope->kacc);
		//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, acc->kcur);
		mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, GLOBALS->minor);
		color = get_normal_color_amount(amount);

		g_object_set(renderer,
			"foreground",  color,
			"text", buf,
			NULL);
	}

	
}


/*
** category cell function
*/
static void ope_category_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
Category *cat;
gchar *fullname;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	if(ope->flags &OF_SPLIT)
	{
		g_object_set(renderer, "text", _("- split -"), NULL);
	}
	else
	{
		cat = da_cat_get(ope->kcat);
		if( cat != NULL )
		{
			fullname = da_cat_get_fullname(cat);
			g_object_set(renderer, "text", fullname, NULL);
			g_free(fullname);
		}
		else
			g_object_set(renderer, "text", "", NULL);

	}
	
}




/* column 3: Infos */
static GtkTreeViewColumn *info_list_transaction_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Info"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, GINT_TO_POINTER(1), NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, GINT_TO_POINTER(2), NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_INFO);

	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}


/* column 4: Payee */
static GtkTreeViewColumn *payee_list_transaction_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_payee_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_PAYEE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}

/* column 5: Wording */
static GtkTreeViewColumn *wording_list_transaction_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Memo"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_wording_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_WORDING);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}




/* column 6/7: Amount/Expense/Income */

static GtkTreeViewColumn *amount_list_transaction_column(gchar *title, gint id)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_amount_cell_data_function, GINT_TO_POINTER(id), NULL);
	gtk_tree_view_column_set_sort_column_id (column, id);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}




/* column 8: Category */
static GtkTreeViewColumn *category_list_transaction_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_category_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_CATEGORY);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}

/* column 9: Tags */
static GtkTreeViewColumn *tags_list_transaction_column()
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Tags"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_tags_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_TAGS);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);

	return column;
}

static GtkTreeViewColumn *list_transaction_get_column(GList *list, gint search_id)
{
GtkTreeViewColumn *column = NULL;
GList *tmp;
gint id;

	tmp = g_list_first(list);
	while (tmp != NULL)
	{
		id = gtk_tree_view_column_get_sort_column_id(tmp->data);
		if( search_id == id)
			column = tmp->data;

		tmp = g_list_next(tmp);
	}

	return column;
}


guint list_transaction_get_quicksearch_column_mask(GtkTreeView *treeview)
{
GtkTreeViewColumn *column;
guint n, mask;
gint id;

	mask = 0;
	for(n=0; n < NUM_LST_DSPOPE-1 ; n++ )   // -1 cause account not to be processed
	{
		column = gtk_tree_view_get_column (treeview, n);
		if(column == NULL)
			continue;

		if( gtk_tree_view_column_get_visible(column) )
		{
			id = gtk_tree_view_column_get_sort_column_id (column);
			switch(id)
			{
				case LST_DSPOPE_WORDING: mask |= FLT_QSEARCH_MEMO; break;
				case LST_DSPOPE_INFO: mask |= FLT_QSEARCH_INFO; break;
				case LST_DSPOPE_PAYEE: mask |= FLT_QSEARCH_PAYEE; break;
				case LST_DSPOPE_CATEGORY: mask |= FLT_QSEARCH_CATEGORY; break;
				case LST_DSPOPE_TAGS: mask |= FLT_QSEARCH_TAGS; break;
			}
		}
	}

	return mask;
}


/* todo: something simpler to sort ? */
void list_transaction_sort_force(GtkTreeSortable *sortable, gpointer user_data)
{
gint sort_column_id;
GtkSortType order;

	DB( g_print("list_transaction_sort_force\n") );

	gtk_tree_sortable_get_sort_column_id(sortable, &sort_column_id, &order);
	DB( g_print(" - id %d order %d\n", sort_column_id, order) );

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, order);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), sort_column_id, order);
}


static void list_transaction_sort_column_changed(GtkTreeSortable *sortable, gpointer user_data)
{
struct list_transaction_data *data = user_data;
gint id;
GtkSortType order;
gboolean showBalance;
	
	gtk_tree_sortable_get_sort_column_id(sortable, &id, &order);

	DB( g_print("list_transaction_columns_changed %d %d\n", id, order) );

	//here save the transaction list columnid and sort order
	PREFS->lst_ope_sort_id    = id;
	PREFS->lst_ope_sort_order = order;

	//manage visibility of balance column
	//showBalance = (id == LST_DSPOPE_DATE && order == GTK_SORT_ASCENDING) ? data->tvc_is_visible : FALSE;
	showBalance = (id == LST_DSPOPE_DATE) ? data->tvc_is_visible : FALSE;
	gtk_tree_view_column_set_visible (data->tvc_balance, showBalance);
}


static void list_transaction_set_columns(GtkTreeView *treeview, gint *visibility)
{
struct list_transaction_data *data;
GtkTreeViewColumn *column, *base;
gboolean visible;
GList *list;
gint i = 0;

	DB( g_print("(list_transaction_set_columns)\n") );

	data = g_object_get_data(G_OBJECT(treeview), "inst_data");
	
	list = gtk_tree_view_get_columns( treeview );

	base = NULL;

	for(i=0; i < NUM_LST_DSPOPE-1 ; i++ )   // -1 cause account not to be processed
	{
		column = list_transaction_get_column(list, ABS(visibility[i]));
		if( column != NULL )
		{
			DB( g_print(" - pos:%d col:%d (%s)\n", i, visibility[i], gtk_tree_view_column_get_title(column)) );

			gtk_tree_view_move_column_after(treeview, column, base);
			base = column;

			visible = visibility[i] < 0 ? FALSE : TRUE;
			gtk_tree_view_column_set_visible (column, visible);
			if( ABS(visibility[i]) == LST_DSPOPE_BALANCE)
			{
				data->tvc_is_visible = visible;
			}
			
		}

	}

	g_list_free(list );
}


static void list_transaction_destroy( GtkWidget *widget, gpointer user_data )
{
struct list_transaction_data *data;

	data = g_object_get_data(G_OBJECT(widget), "inst_data");

	DB( g_print ("\n[list_transaction] destroy event occurred\n") );

	
	DB( g_print(" - view=%p, inst_data=%p\n", widget, data) );
	g_free(data);
}


/*
** create our transaction list
** Status, Date, Info, Payee, Wording, (Amount), Expense, Income, Category
*/
GtkWidget *create_list_transaction(gint type, gboolean *pref_columns)
{
struct list_transaction_data *data;
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column, *col_acc = NULL;


	data = g_malloc0(sizeof(struct list_transaction_data));
	if(!data) return NULL;

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_DSPOPE,
		G_TYPE_POINTER,	/*only really used columns, other are fake */
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
	    G_TYPE_BOOLEAN, G_TYPE_BOOLEAN
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	//store our window private data
	g_object_set_data(G_OBJECT(view), "inst_data", (gpointer)data);
	DB( g_print(" - view=%p, inst_data=%p\n", view, data) );

	// connect our dispose function
	g_signal_connect (view, "destroy",
		G_CALLBACK (list_transaction_destroy), (gpointer)data);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), PREFS->rules_hint);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	if(type == TRN_LIST_TYPE_BOOK)
		gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_MULTIPLE);

	/* column 1: Status */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Status"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_status_cell_data_function, GINT_TO_POINTER(1), NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_status_cell_data_function, GINT_TO_POINTER(2), NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_status_cell_data_function, GINT_TO_POINTER(3), NULL);

	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_STATUS);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	if(type == TRN_LIST_TYPE_DETAIL)
	{
		column = gtk_tree_view_column_new();
		col_acc = column;
		gtk_tree_view_column_set_title(column, _("Account"));
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ope_account_cell_data_function, NULL, NULL);
		gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_ACCOUNT);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	}


	/* column 2: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_date_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	column = info_list_transaction_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = payee_list_transaction_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = wording_list_transaction_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_transaction_column(_("Amount"), LST_DSPOPE_AMOUNT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_transaction_column(_("Expense"), LST_DSPOPE_EXPENSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = amount_list_transaction_column(_("Income"), LST_DSPOPE_INCOME);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = category_list_transaction_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	column = tags_list_transaction_column();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	if(type != TRN_LIST_TYPE_DETAIL)
	{
		column = amount_list_transaction_column(_("Balance"), LST_DSPOPE_BALANCE);
		data->tvc_balance = column;
		gtk_tree_view_column_set_clickable(column, FALSE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	}
	
  /* column 9: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

  /* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_STATUS  , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_STATUS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_DATE    , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_DATE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INFO    , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INFO), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_PAYEE   , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_PAYEE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_WORDING , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_WORDING), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_AMOUNT  , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_AMOUNT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_EXPENSE , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_EXPENSE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INCOME  , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INCOME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_CATEGORY, ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_CATEGORY), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_TAGS    , ope_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_TAGS), NULL);



  /* apply user preference for columns */
	list_transaction_set_columns(GTK_TREE_VIEW(view), pref_columns);

  /* force accoutn column for detail view */
	if(type == TRN_LIST_TYPE_DETAIL)
	{
		gtk_tree_view_move_column_after(GTK_TREE_VIEW(view), col_acc, NULL);
	}

  /* set initial sort order */
    DB( g_print("set sort to %d %d\n", PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order) );
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order);


	/* signals */
	if(type == TRN_LIST_TYPE_BOOK)
		g_signal_connect (GTK_TREE_SORTABLE(store), "sort-column-changed", G_CALLBACK (list_transaction_sort_column_changed), data);

	return(view);
}


/* ---------------------------------------------- */

/*
** account cell function
*/
static void ope_importaccount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
Account *acc, *dacc;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	acc = da_acc_get(ope->kacc);
	if( acc )
	{
		if(acc->imp_key > 0)
		{
			dacc = da_acc_get(acc->imp_key);
			if( dacc )
				g_object_set(renderer, "text", dacc->name, NULL);
		}
		else
			g_object_set(renderer, "text", acc->name, NULL);

	}
}

/*
** amount cell function
*/
static void ope_importamount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *color;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	mystrfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, GLOBALS->minor);
	//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, GLOBALS->minor);

	color = get_normal_color_amount(ope->amount);

	g_object_set(renderer,
			"foreground",  color,
			"text", buf,
			NULL);

}

/*
**
*/
static void ope_importstatus_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
GdkPixbuf *pixbuf = NULL;

	// get the transaction
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	if( ope->same != NULL )
		pixbuf = GLOBALS->lst_pixbuf[LST_PIXBUF_WARNING];

	g_object_set(renderer, "pixbuf", pixbuf, NULL);
}


static void
ope_importfixed_toggled (GtkCellRendererToggle *cell,
	       gchar                 *path_str,
	       gpointer               data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreeIter  iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
  gboolean fixed;

  /* get toggled iter */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, LST_OPE_IMPTOGGLE, &fixed, -1);

  /* do something with the value */
  fixed ^= 1;

  /* set new value */
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, LST_OPE_IMPTOGGLE, fixed, -1);

  /* clean up */
  gtk_tree_path_free (path);
}

/*
** create our transaction list
*/
GtkWidget *create_list_import_transaction(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	/* create list store */
	store = gtk_list_store_new(
	  	10,
		G_TYPE_POINTER,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (view), TRUE);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	//gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_MULTIPLE);

	/* column: Selection */
	renderer = gtk_cell_renderer_toggle_new ();

	g_signal_connect (renderer, "toggled",
		    G_CALLBACK (ope_importfixed_toggled), store);


	column = gtk_tree_view_column_new_with_attributes (NULL,
						     renderer,
						     "active", LST_OPE_IMPTOGGLE,
						     NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Duplicate */
	column = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title(column, _("Import ?"));
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importstatus_cell_data_function, NULL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

//#ifndef NOOFX
	/* column: Account */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Account"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importaccount_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
//#endif

	/* column: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_date_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


	/* column: Wording */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Memo"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_wording_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_WORDING);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Amount"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importamount_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_alignment (column, 1.0);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_EXPENSE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Infos */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Info"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, GINT_TO_POINTER(1), NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_info_cell_data_function, GINT_TO_POINTER(2), NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_INFO);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Payee */

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Payee"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_payee_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_PAYEE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column : Category */

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Category"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_category_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_CATEGORY);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);



	/* column 6: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	return(view);
}
