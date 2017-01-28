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



gchar *list_txn_column_label[] = {
	"----", //datas
	"----", //status
	"----", //date
	N_("Info"    ),
	N_("Payee"   ),
	N_("Memo"    ),
	N_("Amount"  ),
	N_("Expense" ),
	N_("Income"  ),
	N_("Category"),
	N_("Tags"    ),
	N_("Balance" ),
	N_("Status"),
	NULL
};


//debug
//extern gboolean minor_active;

/* This is not pretty. Of course you can also use a
   *  separate compare function for each sort ID value */

static gint list_txn_sort_iter_compare_strings(gchar *s1, gchar *s2)
{
	return hb_string_utf8_compare(s1, s2);
}


static   gint
  list_txn_sort_iter_compare_func (GtkTreeModel *model,
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
			if(!(retval = (ope1->flags & OF_ADDED) - (ope2->flags & OF_ADDED) ) )
			{
				retval = (ope1->flags & OF_CHANGED) - (ope2->flags & OF_CHANGED);
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
					retval = list_txn_sort_iter_compare_strings(a1->name, a2->name);
				}
			}
			break;

		case LST_DSPOPE_INFO:
			if(!(retval = ope1->paymode - ope2->paymode))
			{
				retval = list_txn_sort_iter_compare_strings(ope1->info, ope2->info);
			}
			break;

		case LST_DSPOPE_PAYEE:
			{
			Payee *p1, *p2;

				p1 = da_pay_get(ope1->kpay);
				p2 = da_pay_get(ope2->kpay);
				if( p1 != NULL && p2 != NULL )
				{
					retval = list_txn_sort_iter_compare_strings(p1->name, p2->name);
				}
			}
			break;

		case LST_DSPOPE_WORDING:
				retval = list_txn_sort_iter_compare_strings(ope1->wording, ope2->wording);
			break;

		case LST_DSPOPE_CLR:
			retval = ope1->status - ope2->status;
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
					retval = list_txn_sort_iter_compare_strings(name1, name2);
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
			retval = list_txn_sort_iter_compare_strings(t1, t2);
			g_free(t1);
			g_free(t2);
		}
		break;

		default:
			g_return_val_if_reached(0);
    }

    return retval;
}

static void list_txn_eval_future(GtkCellRenderer *renderer, Transaction *txn)
{
	
	//it seems we are not able to get a valid GdkRGBA
	//nor to set/use the alpha
	/*GdkRGBA *rgba;	
	g_object_get(renderer, "foreground-rgba", &rgba, NULL);
	g_print("forcol: %p %f %f %f %f\n", rgba, rgba->red, rgba->green, rgba->blue, rgba->alpha);
	rgba->red = 1.0;
	rgba->alpha = 0.1;
	g_object_set(renderer, "foreground-set", TRUE, 
		"foreground-rgba", rgba,
	NULL);
	gdk_rgba_free(rgba);
	*/

	if(txn->date > GLOBALS->today)
	{
		g_object_set(renderer, 
			"scale-set", TRUE, 
			"scale", 0.8,
			"style-set", TRUE,
			"style",	PANGO_STYLE_OBLIQUE,
		NULL);
	}
	else
	{
		g_object_set(renderer, "scale-set", FALSE, "style-set", FALSE,
		NULL);
	}
}


/*
** date cell function
*/
static void list_txn_status_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *entry;
gchar *iconname = NULL;

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
			iconname = ( entry->flags & OF_AUTO  ) ? ICONNAME_HB_OPE_AUTO : ( entry->flags & OF_ADDED ) ? ICONNAME_NEW : NULL;
			break;
		case 2:
			iconname = ( entry->flags & OF_CHANGED  ) ? ICONNAME_HB_OPE_EDIT : NULL;
			break;
		/*case 3:
			if( entry->flags & OF_VALID )
				iconname = ICONNAME_HB_OPE_VALID;
			else
			{
				if( entry->flags & OF_REMIND )
					iconname = ICONNAME_HB_OPE_REMIND;
			}
			break;*/
	}
	g_object_set(renderer, "icon-name", iconname, NULL);
}


/*
** account cell function
*/
static void list_txn_account_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
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
static void list_txn_date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar buffer[256];
GDate date;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);

	if(ope->date > 0)
	{
		g_date_set_julian (&date, ope->date);
		g_date_strftime (buffer, 256-1, PREFS->date_format, &date);

		//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

    	g_object_set(renderer, "text", buffer, NULL);
    }
    else
    	g_object_set(renderer, "text", "????", NULL);
}

/*
** info cell function
*/
static void list_txn_info_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	switch(GPOINTER_TO_INT(user_data))
	{
		case 1:
			g_object_set(renderer, "icon-name", get_paymode_icon_name(ope->paymode), NULL);
			break;
		case 2:
			list_txn_eval_future(renderer, ope);
			g_object_set(renderer, "text", ope->info, NULL);
			break;
	}
}

/*
** payee cell function
*/
static void list_txn_payee_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);

	//#926782
	if(ope->paymode == PAYMODE_INTXFER)
	{
	Account *acc = da_acc_get(ope->kxferacc);
		
		g_object_set(renderer, "text", (acc != NULL) ? acc->name : "", NULL);
	}
	else
	{
	Payee *pay = da_pay_get(ope->kpay);
		
		g_object_set(renderer, "text", pay != NULL ? pay->name : "", NULL);
	}
}

/*
** tags cell function
*/
static void list_txn_tags_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar *str;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);

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
static void list_txn_wording_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);
	
    g_object_set(renderer, "text", ope->wording, NULL);
}


/*
** clr cell function
*/
static void list_txn_clr_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar *iconname = NULL;
//const gchar *c = "";
	
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	switch(ope->status)
	{
		/*case TXN_STATUS_CLEARED: c = "c"; break;
		case TXN_STATUS_RECONCILED: c = "R"; break;
		case TXN_STATUS_REMIND: c = "!"; break;*/
		case TXN_STATUS_CLEARED:	iconname = ICONNAME_HB_OPE_CLEARED; break;
		case TXN_STATUS_RECONCILED: iconname = ICONNAME_HB_OPE_RECONCILED; break;
		case TXN_STATUS_REMIND:     iconname = ICONNAME_HB_OPE_REMIND; break;
		
	}

	//g_object_set(renderer, "text", c, NULL);
	g_object_set(renderer, "icon-name", iconname, NULL);
	
}


/*
** amount cell function
*/
static void list_txn_amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gint column = GPOINTER_TO_INT(user_data);
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gint type;
gdouble amount;
gchar *color;

	// get the transaction
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);

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
	//{
		hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, ope->kcur, GLOBALS->minor);
		color = get_normal_color_amount(amount);

		g_object_set(renderer,
			"foreground",  color,
			"text", buf,
			NULL);
	//}

	
}


/*
** category cell function
*/
static void list_txn_category_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
Category *cat;
gchar *fullname;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);
	list_txn_eval_future(renderer, ope);

	if(ope->flags & OF_SPLIT)
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


/* = = = = = = = = = = = = = = = = */
gboolean list_txn_column_id_isvisible(GtkTreeView *treeview, gint sort_id)
{
GtkTreeViewColumn *column;
gint n, id;

	for(n=0; n < NUM_LST_DSPOPE-1 ; n++ )   // -1 cause account not to be processed
	{
		column = gtk_tree_view_get_column (treeview, n);
		if(column == NULL)
			continue;

		if( gtk_tree_view_column_get_visible(column) )
		{
			id = gtk_tree_view_column_get_sort_column_id (column);
			if( sort_id == id )
				return TRUE;
		}
	}

	return FALSE;
}


static GtkTreeViewColumn *list_txn_get_column(GList *list, gint search_id)
{
GtkTreeViewColumn *column = NULL;
GList *tmp;
gint id;

	tmp = g_list_first(list);
	while (tmp != NULL)
	{
		id = gtk_tree_view_column_get_sort_column_id(tmp->data);
		if( search_id == id)
		{
			column = tmp->data;
			break;
		}
		tmp = g_list_next(tmp);
	}
	return column;
}


guint list_txn_get_quicksearch_column_mask(GtkTreeView *treeview)
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


void list_txn_set_save_column_width(GtkTreeView *treeview, gboolean save_column_width)
{
struct list_txn_data *data;

	data = g_object_get_data(G_OBJECT(treeview), "inst_data");
	if( data )
	{
		data->save_column_width = save_column_width;
	}
}


void list_txn_sort_force(GtkTreeSortable *sortable, gpointer user_data)
{
gint sort_column_id;
GtkSortType order;

	DB( g_print("list_txn_sort_force\n") );

	gtk_tree_sortable_get_sort_column_id(sortable, &sort_column_id, &order);
	DB( g_print(" - id %d order %d\n", sort_column_id, order) );

	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, order);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(sortable), sort_column_id, order);
}


static void list_txn_get_columns(GtkTreeView *treeview)
{
struct list_txn_data *data;
GtkTreeViewColumn *column;
gint i, col_id;

	DB( g_print("\n[list_txn] get columns order/width\n") );
	
	data = g_object_get_data(G_OBJECT(treeview), "inst_data");

	DB( g_print(" nbcol=%d, nbsortid=%d\n", NUM_LST_DSPOPE, gtk_tree_view_get_n_columns (treeview)) );
	
	for(i=0 ; i < NUM_LST_DSPOPE-1 ; i++ )   // -1 'caus: account and blank column
	{
		column = gtk_tree_view_get_column(treeview, i);
		if(column != NULL)
		{
			col_id = gtk_tree_view_column_get_sort_column_id (column);
			if( col_id >= 0 )
			{
			gboolean visible;

				visible = gtk_tree_view_column_get_visible (column);
				if( col_id == LST_DSPOPE_BALANCE)   //keep initial state of balance
					visible = data->tvc_is_visible;

				if( visible )
					PREFS->lst_ope_columns[i] = col_id;
				else
					PREFS->lst_ope_columns[i] = -col_id;
				
				PREFS->lst_ope_col_size[col_id-1] = gtk_tree_view_column_get_width(column);

				DB( g_print(" col-%2d => %2d '%s' w=%d\n", i, col_id, gtk_tree_view_column_get_title(column), PREFS->lst_ope_col_size[col_id-1] ) );
			}
			else	//should not occurs
				PREFS->lst_ope_columns[i] = 0;
		}
	}
}


static void list_txn_set_columns(GtkTreeView *treeview, gint *col_id)
{
struct list_txn_data *data;
GtkTreeViewColumn *column, *base;
gboolean visible;
GList *list;
gint i = 0;
gint id;

	DB( g_print("\n[list_txn] set columns order/width\n") );
	
	data = g_object_get_data(G_OBJECT(treeview), "inst_data");

#if MYDEBUG == 1
	DB( g_print("\n debug column sortid\n") );

	for(i=0; i < NUM_LST_DSPOPE-1 ; i++ )   // -1 cause account not to be processed
	{
		DB( g_print(" - pos:%2d sortid:%2d\n", i, col_id[i]) );
	}
#endif	


	DB( g_print("\n apply column prefs\n") );

	list = gtk_tree_view_get_columns( treeview );

	base = NULL;

	for(i=0; i < NUM_LST_DSPOPE-1 ; i++ )   // -1 cause account not to be processed
	{
		/* hidden are stored as col_id negative */
		id = ABS(col_id[i]);
		column = list_txn_get_column(list, id);

		//DB( g_print(" - get column %d %p\n", id, column) );
		
		if( column != NULL )
		{
			DB( g_print(" - pos:%2d sortid:%2d (%s)\n", i, col_id[i], gtk_tree_view_column_get_title(column)) );

			gtk_tree_view_move_column_after(treeview, column, base);
			base = column;

			visible = col_id[i] < 0 ? FALSE : TRUE;
			
			/* display exception for detail/import list */
			if(data->list_type != LIST_TXN_TYPE_BOOK)
			{
				if( id == LST_DSPOPE_AMOUNT )
					visible = TRUE;
				
				if( id == LST_DSPOPE_STATUS || id == LST_DSPOPE_EXPENSE || id == LST_DSPOPE_INCOME )
					visible = FALSE;
			}
			
			gtk_tree_view_column_set_visible (column, visible);
			if( id == LST_DSPOPE_BALANCE )
			{
				data->tvc_is_visible = visible;
			}

			if(   id == LST_DSPOPE_INFO
			   || id == LST_DSPOPE_PAYEE
			   || id == LST_DSPOPE_WORDING
			   || id == LST_DSPOPE_CATEGORY
			   || id == LST_DSPOPE_TAGS
			   || id == LST_DSPOPE_ACCOUNT )
			{
				gtk_tree_view_column_set_fixed_width( column, PREFS->lst_ope_col_size[id - 1]);
			}
		}

	}

	g_list_free(list );
}


static void list_txn_sort_column_changed(GtkTreeSortable *sortable, gpointer user_data)
{
struct list_txn_data *data = user_data;
gint id;
GtkSortType order;
gboolean showBalance;
	
	gtk_tree_sortable_get_sort_column_id(sortable, &id, &order);

	DB( g_print("list_txn_columns_changed %d %d\n", id, order) );

	//here save the transaction list columnid and sort order
	PREFS->lst_ope_sort_id    = id;
	PREFS->lst_ope_sort_order = order;

	//manage visibility of balance column
	//showBalance = (id == LST_DSPOPE_DATE && order == GTK_SORT_ASCENDING) ? data->tvc_is_visible : FALSE;
	showBalance = (id == LST_DSPOPE_DATE) ? data->tvc_is_visible : FALSE;
	gtk_tree_view_column_set_visible (data->tvc_balance, showBalance);
}


static void
list_txn_column_popup_menuitem_on_activate (GtkCheckMenuItem *checkmenuitem,
               gpointer          user_data)
{
GtkTreeViewColumn *column = user_data;

	DB( g_print("toggled\n") );

	gtk_tree_view_column_set_visible(column, gtk_check_menu_item_get_active(checkmenuitem) );
}


static gboolean list_txn_column_popup_callback ( GtkWidget *button,
                        GdkEventButton *ev,
                        gpointer user_data )
{
struct list_txn_data *data = user_data;
GtkWidget *menu, *menuitem;
GtkTreeViewColumn *column;
gint i, col_id;

 
	if( ev->button == 3 )
	{
		DB( g_print("should popup\n") );
	
		menu = gtk_menu_new ();
		
		//note: deactive this disable any menuitem action
		g_signal_connect (menu, "selection-done", G_CALLBACK (gtk_widget_destroy), NULL);

		for(i=0 ; i < NUM_LST_DSPOPE-1 ; i++ )   // -1 'caus: account and blank column
		{
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(data->treeview), i);
			if( column != NULL )
			{
				col_id = gtk_tree_view_column_get_sort_column_id (column);

				if( (col_id == -1) 
					|| (col_id == LST_DSPOPE_STATUS) 
					|| (col_id == LST_DSPOPE_ACCOUNT) 
					|| (col_id == LST_DSPOPE_DATE)
					|| (col_id == LST_DSPOPE_BALANCE)
				)
					continue;
				//if( (data->tvc_is_visible == FALSE) && (col_id == LST_DSPOPE_BALANCE) )
				//	continue;
				
				if( (data->list_type == LIST_TXN_TYPE_DETAIL) && 
					(   (col_id == LST_DSPOPE_AMOUNT) 
					|| (col_id == LST_DSPOPE_EXPENSE) 
					|| (col_id == LST_DSPOPE_INCOME)
					) 
				)
					continue;

				menuitem = gtk_check_menu_item_new_with_label ( gtk_tree_view_column_get_title (column) );
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), gtk_tree_view_column_get_visible (column) );
				gtk_widget_show (menuitem);
				
				g_signal_connect (menuitem, "activate",
								G_CALLBACK (list_txn_column_popup_menuitem_on_activate), column);
			}
		
		}
	
		gtk_menu_attach_to_widget (GTK_MENU (menu), button, NULL);
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
				      ev->button, ev->time);
	}

    return FALSE;
}



static GtkTreeViewColumn *
list_txn_column_amount_create(gint list_type, gchar *title, gint sortcolumnid, GtkTreeCellDataFunc func)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);

	column = gtk_tree_view_column_new_with_attributes(title, renderer, NULL);
	
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, sortcolumnid);
	if(list_type == LIST_TXN_TYPE_BOOK)
	{
		gtk_tree_view_column_set_reorderable(column, TRUE);
	}
	gtk_tree_view_column_set_cell_data_func(column, renderer, func, GINT_TO_POINTER(sortcolumnid), NULL);
		
	return column;
}


static GtkTreeViewColumn *
list_txn_column_text_create(gint list_type, gchar *title, gint sortcolumnid, GtkTreeCellDataFunc func, gpointer user_data)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	renderer = gtk_cell_renderer_text_new ();
	/*g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);*/
	
	column = gtk_tree_view_column_new_with_attributes(title, renderer, NULL);

	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);

	gtk_tree_view_column_set_sort_column_id (column, sortcolumnid);
	if(list_type == LIST_TXN_TYPE_BOOK)
	{
		gtk_tree_view_column_set_reorderable(column, TRUE);
		gtk_tree_view_column_set_min_width (column, HB_MINWIDTH_COLUMN);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	}
	gtk_tree_view_column_set_cell_data_func(column, renderer, func, user_data, NULL);

	return column;
}


static GtkTreeViewColumn *
list_txn_column_info_create(gint list_type)
{
GtkTreeViewColumn  *column;
GtkCellRenderer    *renderer;

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Info"));

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_info_cell_data_function, GINT_TO_POINTER(1), NULL);

	renderer = gtk_cell_renderer_text_new ();
	/*g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);*/
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_info_cell_data_function, GINT_TO_POINTER(2), NULL);

	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_INFO);
	if(list_type == LIST_TXN_TYPE_BOOK)
	{
		gtk_tree_view_column_set_reorderable(column, TRUE);
		gtk_tree_view_column_set_min_width (column, HB_MINWIDTH_COLUMN);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	}

	return column;
}


static void list_txn_destroy( GtkWidget *widget, gpointer user_data )
{
struct list_txn_data *data;

	data = g_object_get_data(G_OBJECT(widget), "inst_data");

	DB( g_print ("\n[list_transaction] destroy event occurred\n") );

	if( data->save_column_width )
	{
		list_txn_get_columns(GTK_TREE_VIEW(data->treeview));
	}
		
	DB( g_print(" - view=%p, inst_data=%p\n", widget, data) );
	g_free(data);
}


Transaction *list_txn_get_active_transaction(GtkTreeView *treeview)
{
GtkTreeModel *model;
GList *list;
Transaction *ope;

	ope = NULL;

	model = gtk_tree_view_get_model(treeview);
	list = gtk_tree_selection_get_selected_rows(gtk_tree_view_get_selection(treeview), &model);

	if(list != NULL)
	{
	GtkTreeIter iter;

		gtk_tree_model_get_iter(model, &iter, list->data);
		gtk_tree_model_get(model, &iter, LST_DSPOPE_DATAS, &ope, -1);
	}

	g_list_foreach(list, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(list);

	return ope;
}


/*
** create our transaction list
** Status, Date, Info, Payee, Wording, (Amount), Expense, Income, Category
*/
GtkWidget *create_list_transaction(gint list_type, gboolean *pref_columns)
{
struct list_txn_data *data;
GtkListStore *store;
GtkWidget *treeview;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column, *col_acc = NULL, *col_status = NULL;


	data = g_malloc0(sizeof(struct list_txn_data));
	if(!data) return NULL;

	data->list_type = list_type;
	data->save_column_width = FALSE;
	
	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_DSPOPE,
		G_TYPE_POINTER,	/*only really used columns, other are fake */
	    G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
		G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
	    G_TYPE_BOOLEAN, G_TYPE_BOOLEAN
		);

	//treeview
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	data->treeview = treeview;
	g_object_unref(store);

	//store our window private data
	g_object_set_data(G_OBJECT(treeview), "inst_data", (gpointer)data);
	DB( g_print(" - treeview=%p, inst_data=%p\n", treeview, data) );

	// connect our dispose function
	g_signal_connect (treeview, "destroy", G_CALLBACK (list_txn_destroy), (gpointer)data);
	
	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	if(list_type == LIST_TXN_TYPE_BOOK)
		gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), GTK_SELECTION_MULTIPLE);

	/* column 1: Changes */
	column = gtk_tree_view_column_new();
	//gtk_tree_view_column_set_title(column, _("Status"));
	col_status = column;

	renderer = gtk_cell_renderer_pixbuf_new ();
	//gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_status_cell_data_function, GINT_TO_POINTER(1), NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	//gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_status_cell_data_function, GINT_TO_POINTER(2), NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	//gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_status_cell_data_function, GINT_TO_POINTER(3), NULL);

	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_STATUS);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	if(list_type == LIST_TXN_TYPE_DETAIL)
	{
		column = list_txn_column_text_create(list_type, _("Account"), LST_DSPOPE_ACCOUNT, list_txn_account_cell_data_function, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
		col_acc = column;
		// add column popup
		g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
			"button-press-event",
			G_CALLBACK ( list_txn_column_popup_callback ),
			data );
	}

	/* column 2: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_date_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );


	column = list_txn_column_info_create(list_type);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );
	
	column = list_txn_column_text_create(list_type, _("Payee"), LST_DSPOPE_PAYEE, list_txn_payee_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	column = list_txn_column_text_create(list_type, _("Memo"), LST_DSPOPE_WORDING, list_txn_wording_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	/* column status CLR */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Status"));
	//renderer = gtk_cell_renderer_text_new ();
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_clr_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_CLR);
	//gtk_tree_view_column_set_sort_indicator (column, FALSE);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );
	
	
	column = list_txn_column_amount_create(list_type, _("Amount"), LST_DSPOPE_AMOUNT, list_txn_amount_cell_data_function);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	column = list_txn_column_amount_create(list_type, _("Expense"), LST_DSPOPE_EXPENSE, list_txn_amount_cell_data_function);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	column = list_txn_column_amount_create(list_type, _("Income"), LST_DSPOPE_INCOME, list_txn_amount_cell_data_function);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	column = list_txn_column_text_create(list_type, _("Category"), LST_DSPOPE_CATEGORY, list_txn_category_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	column = list_txn_column_text_create(list_type, _("Tags"), LST_DSPOPE_TAGS, list_txn_tags_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	// add column popup
    g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
		"button-press-event",
		G_CALLBACK ( list_txn_column_popup_callback ),
		data );

	if(list_type == LIST_TXN_TYPE_BOOK)
	{
		column = list_txn_column_amount_create(list_type, _("Balance"), LST_DSPOPE_BALANCE, list_txn_amount_cell_data_function);
		data->tvc_balance = column;
		gtk_tree_view_column_set_clickable(column, FALSE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
		// add column popup
		g_signal_connect ( G_OBJECT (gtk_tree_view_column_get_button (column)), 
			"button-press-event",
			G_CALLBACK ( list_txn_column_popup_callback ),
			data );
	}
	
  /* column 9: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

  /* sort */
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_STATUS  , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_STATUS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_DATE    , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_DATE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INFO    , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INFO), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_PAYEE   , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_PAYEE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_WORDING , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_WORDING), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_AMOUNT  , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_AMOUNT), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_EXPENSE , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_EXPENSE), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_INCOME  , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_INCOME), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_CATEGORY, list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_CATEGORY), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_TAGS    , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_TAGS), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_CLR     , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_CLR), NULL);
	gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), LST_DSPOPE_ACCOUNT , list_txn_sort_iter_compare_func, GINT_TO_POINTER(LST_DSPOPE_ACCOUNT), NULL);

  /* apply user preference for columns */
	list_txn_set_columns(GTK_TREE_VIEW(treeview), pref_columns);

  /* force account column for detail treeview */
	if(list_type == LIST_TXN_TYPE_DETAIL)
	{
		gtk_tree_view_move_column_after(GTK_TREE_VIEW(treeview), col_acc, col_status);
	}

  /* set initial sort order */
    DB( g_print("set sort to %d %d\n", PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order) );
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), PREFS->lst_ope_sort_id, PREFS->lst_ope_sort_order);


	/* signals */
	if(list_type == LIST_TXN_TYPE_BOOK)
		g_signal_connect (GTK_TREE_SORTABLE(store), "sort-column-changed", G_CALLBACK (list_txn_sort_column_changed), data);

	return(treeview);
}


/* ---------------------------------------------- */
/*  import list : soon obsolete */
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
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gchar *color;

	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	//mystrfnum(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, GLOBALS->minor);
	//hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, ope->amount, GLOBALS->minor);
	g_ascii_formatd(formatd_buf, G_ASCII_DTOSTR_BUF_SIZE-1, "%.2f", ope->amount);

	color = get_normal_color_amount(ope->amount);

	g_object_set(renderer,
			"foreground",  color,
			"text", formatd_buf,
			NULL);

}


static void ope_importstatus_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Transaction *ope;
gchar *iconname = NULL;

	// get the transaction
	gtk_tree_model_get(model, iter, LST_DSPOPE_DATAS, &ope, -1);

	iconname = ( ope->same != NULL ) ? ICONNAME_WARNING : NULL;

	g_object_set(renderer, "icon-name", iconname, NULL);
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
GtkWidget *create_list_import_transaction(gboolean enable_choose)
{
GtkListStore *store;
GtkWidget *treeview;
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
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (treeview), PREFS->grid_lines);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	//gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_MULTIPLE);

	/* column: Selection */
	if(enable_choose)
	{
		renderer = gtk_cell_renderer_toggle_new ();

		g_signal_connect (renderer, "toggled", G_CALLBACK (ope_importfixed_toggled), store);


		column = gtk_tree_view_column_new_with_attributes (NULL,
								 renderer,
								 "active", LST_OPE_IMPTOGGLE,
								 NULL);

		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

		/* column: Duplicate */
		column = gtk_tree_view_column_new();
		//gtk_tree_view_column_set_title(column, _("Import ?"));
		renderer = gtk_cell_renderer_pixbuf_new ();
		//gtk_cell_renderer_set_fixed_size(renderer, GLOBALS->lst_pixbuf_maxwidth, -1);
		gtk_tree_view_column_pack_start(column, renderer, TRUE);
		gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importstatus_cell_data_function, NULL, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
	}
		
//#ifndef NOOFX
	/* column: Account */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Account"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, ope_importaccount_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);
//#endif

	/* column: Date */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Date"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, list_txn_date_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = list_txn_column_text_create(LIST_TXN_TYPE_IMPORT, _("Memo"), -1, list_txn_wording_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = list_txn_column_amount_create(LIST_TXN_TYPE_IMPORT, _("Amount"), -1, ope_importamount_cell_data_function);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = list_txn_column_info_create(LIST_TXN_TYPE_IMPORT);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = list_txn_column_text_create(LIST_TXN_TYPE_IMPORT, _("Payee"), -1, list_txn_payee_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = list_txn_column_text_create(LIST_TXN_TYPE_IMPORT, _("Category"), -1, list_txn_category_cell_data_function, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

	return(treeview);
}
