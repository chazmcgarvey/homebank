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

#include "list-scheduled.h"

/****************************************************************************/
/* Debug macros																*/
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


static void
sched_lateicon_cell_data_function (GtkTreeViewColumn *col,
                           GtkCellRenderer   *renderer,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           user_data)
{
gchar *iconname = NULL;
gint nblate;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_NB_LATE, &nblate,
		-1);

	iconname = ( nblate > 0 ) ? ICONNAME_WARNING : NULL;

	g_object_set(renderer, "icon-name", iconname, NULL);
}


/*
** remaining cell function
*/
static void sched_latetext_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
gchar *markuptxt;
gchar *color;
gint nblate;
//gint weight;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
	    LST_DSPUPC_NB_LATE, &nblate,
		-1);

	if(arc && nblate > 0)
	{
		markuptxt = g_strdup_printf(nblate < 10 ? "%d" : "+10", nblate);

		color = NULL;
		//weight = PANGO_WEIGHT_NORMAL;

		if(nblate > 0 && PREFS->custom_colors == TRUE)
		{
			color = PREFS->color_warn;
		}

		g_object_set(renderer,
			//"weight", weight,
			"foreground",  color,
			"text", markuptxt,
			NULL);

		g_free(markuptxt);
	}
	else
		g_object_set(renderer, "text", NULL, NULL);

}

/*
** date cell function
*/
static void date_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
gchar buffer[256];
GDate *date;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
		-1);

	if(arc)
	{
		date = g_date_new_julian (arc->nextdate);
		g_date_strftime (buffer, 256-1, PREFS->date_format, date);
		g_date_free(date);

		//g_snprintf(buf, sizeof(buf), "%d", ope->ope_Date);

		g_object_set(renderer, "text", buffer, NULL);

	}
		else
		g_object_set(renderer, "text", NULL, NULL);

}


/*
** still cell function
*/

static void still_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
gchar *markuptxt;

	gtk_tree_model_get(model, iter,
	    LST_DSPUPC_DATAS, &arc,
		-1);

	if(arc && (arc->flags & OF_LIMIT) )
	{
		markuptxt = g_strdup_printf("%d", arc->limit);
		g_object_set(renderer, "markup", markuptxt, NULL);
		g_free(markuptxt);
	}
	else
		g_object_set(renderer, "text", NULL, NULL);
}


/*
** payee cell function
*/
static void payee_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
Payee *pay;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
		-1);

	if(arc)
	{

		pay = da_pay_get(arc->kpay);

		if(pay != NULL)
			g_object_set(renderer, "text", pay->name, NULL);
	}
		else
		g_object_set(renderer, "text", NULL, NULL);

}

/*
** memo cell function
*/
static void memo_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
gchar *memo;
gint weight;

	gtk_tree_model_get(model, iter,
		LST_DSPUPC_DATAS, &arc,
		LST_DSPUPC_MEMO, &memo,
		-1);

	weight = arc == NULL ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

	g_object_set(renderer, "weight", weight, "text", memo, NULL);

	//leak
	g_free(memo);

}


/*
** amount cell function
*/
static void amount_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Archive *arc;
gdouble expense, income, amount;
gchar buf[G_ASCII_DTOSTR_BUF_SIZE];
gint column = GPOINTER_TO_INT(user_data);
Account *acc;
gchar *color;
gint weight;

	gtk_tree_model_get(model, iter, 
	    LST_DSPUPC_DATAS, &arc,
		LST_DSPUPC_ACCOUNT, &acc,
		LST_DSPUPC_EXPENSE, &expense,
	    LST_DSPUPC_INCOME, &income,
	    -1);

	amount = column == -1 ? expense : income;
		
	if( amount != 0.0)
	{
		if( acc != NULL )
			hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, acc->kcur, GLOBALS->minor);
		else
			hb_strfmon(buf, G_ASCII_DTOSTR_BUF_SIZE-1, amount, GLOBALS->kcur, GLOBALS->minor);

		color = get_normal_color_amount(amount);

		weight = arc == NULL ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

		g_object_set(renderer,
			"weight", weight,
			"foreground",  color,
			"text", buf,
			NULL);
	}
	else
	{
		g_object_set(renderer, "text", NULL, NULL);
	}
	
}

/*
** account cell function
*/
static void account_cell_data_function (GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
Account *acc;
gchar *name = NULL;

	gtk_tree_model_get(model, iter, 
		LST_DSPUPC_ACCOUNT, &acc,
		-1);
	/* 1378836 display acc or dstacc */
	if( acc != NULL )
	{
		name = acc->name;
	}

	g_object_set(renderer, "text", name, NULL);

}


static
gboolean list_account_selectionfunc(
GtkTreeSelection *selection, GtkTreeModel *model, GtkTreePath *path, gboolean path_currently_selected, gpointer data)
{
GtkTreeIter iter;
Archive *arc;

	if(gtk_tree_model_get_iter(model, &iter, path))
	{
		gtk_tree_model_get(model, &iter,
			LST_DSPUPC_DATAS, &arc,
		    -1);

		if( arc == NULL )
			return FALSE;
	}

	return TRUE;
}


static GtkTreeViewColumn *list_upcoming_get_column(GtkTreeView *treeview, gint uid)
{
GtkTreeViewColumn *retval = NULL;
guint i;

	for(i=0;i<gtk_tree_view_get_n_columns(treeview);i++)
	{
	GtkTreeViewColumn *column = gtk_tree_view_get_column(treeview, i);

		if(column)
		{
			gint coluid = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "uid"));
			if( coluid == uid )
				retval = column;
		}
	}
	return retval;	
}


static void list_upcoming_destroy(GtkTreeView *treeview, gpointer user_data)
{
GtkTreeViewColumn *column;

	DB( g_print ("\n[list_upcoming] destroy\n") );

	//#1830656 use xxx_get_fixed_width instead of width, as if not visible will save 0 otherwise
	column = list_upcoming_get_column(treeview, COL_DSPUPC_PAYEE);
	if( column )
	{
		PREFS->pnl_upc_col_pay_width = gtk_tree_view_column_get_fixed_width(column);
	}
	
	column = list_upcoming_get_column(treeview, COL_DSPUPC_MEMO);
	if( column )
	{
		PREFS->pnl_upc_col_mem_width = gtk_tree_view_column_get_fixed_width(column);
	}

	DB( g_print(" width payee:%d, memo:%d\n", PREFS->pnl_upc_col_pay_width, PREFS->pnl_upc_col_mem_width) );

}


GtkWidget *create_list_upcoming(void)
{
GtkListStore *store;
GtkWidget *view;
GtkCellRenderer    *renderer;
GtkTreeViewColumn  *column;

	DB( g_print ("\n[list_upcoming] create\n") );

	/* create list store */
	store = gtk_list_store_new(
	  	NUM_LST_DSPUPC,
		G_TYPE_POINTER,	/* scheduled */
	    G_TYPE_INT,		/* next (sort) */
		G_TYPE_POINTER,	/* account */
	    G_TYPE_STRING,	/* memo/total */
		G_TYPE_DOUBLE,	/* expense */
		G_TYPE_DOUBLE,	/* income */
		G_TYPE_INT		/* nb late */
		);

	//treeview
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(store);

	gtk_tree_view_set_grid_lines (GTK_TREE_VIEW (view), PREFS->grid_lines);
	//gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),
	//			       COLUMN_DESCRIPTION);

	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);

	/* column : Late */
	column = gtk_tree_view_column_new();
	//TRANSLATORS: title of list column to inform the scheduled transaction is Late
	gtk_tree_view_column_set_title(column, _("Late"));

	renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, sched_lateicon_cell_data_function, NULL, NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, sched_latetext_cell_data_function, NULL, NULL);

	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPUPC_NB_LATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column : Still (for limited scheduled) */
	column = gtk_tree_view_column_new();
	//TRANSLATORS: title of list column to inform how many occurence remain to post for limited scheduled txn
	gtk_tree_view_column_set_title(column, _("Still"));

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, still_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPUPC_REMAINING);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);
	
	/* column: Next on */
	renderer = gtk_cell_renderer_text_new ();

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Next date"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, date_cell_data_function, NULL, NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPUPC_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Payee */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);
	column = gtk_tree_view_column_new();
	g_object_set_data(G_OBJECT(column), "uid", GUINT_TO_POINTER(COL_DSPUPC_PAYEE));
	gtk_tree_view_column_set_title(column, _("Payee"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, payee_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST/2);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_column_set_fixed_width(column, PREFS->pnl_upc_col_pay_width);

	/* column: Memo */
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);

	column = gtk_tree_view_column_new();
	g_object_set_data(G_OBJECT(column), "uid", GUINT_TO_POINTER(COL_DSPUPC_MEMO));
	gtk_tree_view_column_set_title(column, _("Memo"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, memo_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_add_attribute(column, renderer, "text", 2);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST/2);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_view_column_set_fixed_width(column, PREFS->pnl_upc_col_mem_width);

	/* column: Amount */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Expense"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, amount_cell_data_function, GINT_TO_POINTER(-1), NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Amount */
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, _("Income"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, amount_cell_data_function, GINT_TO_POINTER(1), NULL);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPACC_NAME);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	/* column: Account */
	renderer = gtk_cell_renderer_text_new ();
	/*g_object_set(renderer, 
		"ellipsize", PANGO_ELLIPSIZE_END,
	    "ellipsize-set", TRUE,
	    NULL);*/

	column = gtk_tree_view_column_new();
	g_object_set_data(G_OBJECT(column), "uid", GUINT_TO_POINTER(COL_DSPUPC_ACCOUNT));
	gtk_tree_view_column_set_title(column, _("Account"));
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, account_cell_data_function, NULL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_set_sort_column_id (column, LST_DSPOPE_DATE);
	gtk_tree_view_column_set_alignment (column, 0.5);
	//gtk_tree_view_column_set_min_width(column, HB_MINWIDTH_LIST);
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);


  /* column: empty */
	column = gtk_tree_view_column_new();
	gtk_tree_view_append_column (GTK_TREE_VIEW(view), column);

	gtk_tree_selection_set_select_function(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), list_account_selectionfunc, NULL, NULL);

	
    /* set initial sort order */
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(store), LST_DSPUPC_NEXT, GTK_SORT_ASCENDING);


	g_signal_connect (view, "destroy", G_CALLBACK (list_upcoming_destroy), NULL);


	return(view);
}

