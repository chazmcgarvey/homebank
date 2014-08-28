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

#include "hb-transaction.h"
#include "hb-tag.h"

/****************************************************************************/
/* Debug macros										 */
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


static void da_split_free(Split *item)
{
	if(item != NULL)
	{
		if(item->memo != NULL)
			g_free(item->memo);

		g_free(item);
	}
}


static Split *da_split_malloc(void)
{
	return g_malloc0(sizeof(Split));
}


Split *da_split_new(guint32 kcat, gdouble amount, gchar	*memo)
{
Split *split = da_split_malloc();

	split->kcat = kcat;
	split->amount = amount;
	split->memo = g_strdup(memo);
	return split;
}



static Split *da_split_clone(Split *src_split)
{
Split *new_split = g_memdup(src_split, sizeof(Split));

	DB( g_print("da_split_clone\n") );

	if(new_split)
	{
		//duplicate the string
		new_split->memo = g_strdup(src_split->memo);
		DB( g_print(" clone %p -> %p\n", src_split, new_split) );

	}
	return new_split;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint da_transaction_splits_count(Transaction *txn)
{
guint i, count = 0;

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		if(txn->splits[i] == NULL)
			break;
		count++;
	}
	return count;
}


void da_transaction_splits_free(Transaction *txn)
{
guint count, i=0;

	count = da_transaction_splits_count(txn);
	if(count == 0)
		return;
	
	DB( g_print("da_transaction_splits_free\n") );

	for(;i<=count;i++)
	{
		DB( g_print("- freeing %d :: %p\n", i, txn->splits[i]) );
		
		da_split_free(txn->splits[i]);
		txn->splits[i] = NULL;
	}
	//remove the flag
	txn->flags &= ~(OF_SPLIT);

}


void da_transaction_splits_append(Transaction *txn, Split *split)
{
guint count = da_transaction_splits_count(txn);

	DB( g_print("da_transaction_splits_append\n") );

	DB( g_print("- split[%d] at %p for ope %p\n", count, split, txn) );

	txn->flags |= OF_SPLIT;
	txn->splits[count] = split;
	txn->splits[count + 1] = NULL;
	
	DB( g_print("- %d splits\n", da_transaction_splits_count(txn)) );
}


void da_transaction_splits_clone(Transaction *stxn, Transaction *dtxn)
{
gint i, count;

	DB( g_print("da_transaction_splits_clone\n") );
	
	count = da_transaction_splits_count(stxn);
	for(i=0;i<count;i++)
	{
		dtxn->splits[i] = da_split_clone(stxn->splits[i]);
	}	

	if(count > 0)
		dtxn->flags |= OF_SPLIT;
	
	DB( g_print(" clone %p -> %p, %d splits\n", stxn, dtxn, count) );
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void
da_transaction_clean(Transaction *item)
{
	if(item != NULL)
	{
		if(item->wording != NULL)
		{
			g_free(item->wording);
			item->wording = NULL;
		}
		if(item->info != NULL)
		{
			g_free(item->info);
			item->info = NULL;
		}
		if(item->tags != NULL)
		{
			DB( g_print(" -> item->tags %p\n", item->tags) );
			g_free(item->tags);
			item->tags = NULL;
		}

		da_transaction_splits_free(item);

		if(item->same != NULL)
		{
			g_list_free(item->same);
			item->same = NULL;
		}
	}
}



void
da_transaction_free(Transaction *item)
{
	if(item != NULL)
	{
		da_transaction_clean(item);
		g_free(item);
	}
}


Transaction *
da_transaction_malloc(void)
{
	return g_malloc0(sizeof(Transaction));
}


Transaction *da_transaction_copy(Transaction *src_txn, Transaction *dst_txn)
{
guint count;

	DB( g_print("da_transaction_copy\n") );

	da_transaction_clean (dst_txn);

	memmove(dst_txn, src_txn, sizeof(Transaction));
	
	//duplicate the string
	dst_txn->wording = g_strdup(src_txn->wording);
	dst_txn->info = g_strdup(src_txn->info);

	//duplicate tags
	dst_txn->tags = NULL;
	count = transaction_tags_count(src_txn);
	if(count > 0)
		dst_txn->tags = g_memdup(src_txn->tags, count*sizeof(guint32));

	da_transaction_splits_clone(src_txn, dst_txn);

	return dst_txn;
}


Transaction *da_transaction_init_from_template(Transaction *txn, Archive *arc)
{
	//txn->date		= 0;
	txn->amount		= arc->amount;
	txn->kacc		= arc->kacc;
	txn->paymode	= arc->paymode;
	txn->flags		= arc->flags | OF_ADDED;
	txn->kpay		= arc->kpay;
	txn->kcat		= arc->kcat;
	txn->kxferacc	= arc->kxferacc;
	txn->wording	= g_strdup(arc->wording);
	txn->info		= NULL;

	return txn;
}


Transaction *da_transaction_clone(Transaction *src_item)
{
Transaction *new_item = g_memdup(src_item, sizeof(Transaction));
guint count;

	DB( g_print("da_transaction_clone\n") );

	if(new_item)
	{
		//duplicate the string
		new_item->wording = g_strdup(src_item->wording);
		new_item->info = g_strdup(src_item->info);

		//duplicate tags
		new_item->tags = NULL;
		count = transaction_tags_count(src_item);
		if(count > 0)
			new_item->tags = g_memdup(src_item->tags, count*sizeof(guint32));

		da_transaction_splits_clone(src_item, new_item);

	}
	return new_item;
}

GList *
da_transaction_new(void)
{
	return NULL;
}


void da_transaction_destroy(GList *list)
{
GList *tmplist = g_list_first(list);

	while (tmplist != NULL)
	{
	Transaction *item = tmplist->data;
		da_transaction_free(item);
		tmplist = g_list_next(tmplist);
	}
	g_list_free(list);
}


static gint da_transaction_compare_func(Transaction *a, Transaction *b)
{
	return ((gint)a->date - b->date);
}


GList *da_transaction_sort(GList *list)
{
	return( g_list_sort(list, (GCompareFunc)da_transaction_compare_func));
}


static void da_transaction_insert_memo(Transaction *item)
{
	// append the memo if new
	if( item->wording != NULL )
	{
		if( g_hash_table_lookup(GLOBALS->h_memo, item->wording) == NULL )
		{
			g_hash_table_insert(GLOBALS->h_memo, g_strdup(item->wording), NULL);
		}
	}
}



gboolean da_transaction_insert_sorted(Transaction *newitem)
{
GList *tmplist = g_list_first(GLOBALS->ope_list);

	// find the breaking date
	while (tmplist != NULL)
	{
	Transaction *item = tmplist->data;

		if(item->date > newitem->date)
			break;

		tmplist = g_list_next(tmplist);
	}

	// here we're at the insert point, let's insert our new txn just before
	GLOBALS->ope_list = g_list_insert_before(GLOBALS->ope_list, tmplist, newitem);

	da_transaction_insert_memo(newitem);
	return TRUE;
}


// nota: this is called only when loading xml file
gboolean da_transaction_prepend(Transaction *item)
{
	GLOBALS->ope_list = g_list_prepend(GLOBALS->ope_list, item);
	da_transaction_insert_memo(item);
	return TRUE;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

guint32
da_transaction_get_max_kxfer(void)
{
guint32 max_key = 0;
GList *list;
Transaction *item;

	DB( g_print("da_transaction_get_max_kxfer\n") );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		item = list->data;
		if( item->paymode == PAYMODE_INTXFER)
		{
			if( item->kxfer > max_key)
				max_key = item->kxfer;
		}
		list = g_list_next(list);
	}

	DB( g_print(" max_key : %d \n", max_key) );


	return max_key;
}

static void da_transaction_goto_orphan(Transaction *txn)
{
const gchar *oatn = "orphaned transactions";
Account *acc;

	acc = da_acc_get_by_name((gchar *)oatn);
	if(acc == NULL)
	{
		acc = da_acc_malloc();
		acc->name = g_strdup(oatn);
		da_acc_append(acc);
	}
	txn->kacc = acc->key;
}


void da_transaction_consistency(Transaction *item)
{
Account *acc;
Category *cat;
Payee *pay;
guint i, nbsplit;

	// check account exists
	acc = da_acc_get(item->kacc);
	if(acc == NULL)
	{
		g_warning("txn consistency: fixed invalid acc %d", item->kacc);
		da_transaction_goto_orphan(item);
	}

	// check category exists
	cat = da_cat_get(item->kcat);
	if(cat == NULL)
	{
		g_warning("txn consistency: fixed invalid cat %d", item->kcat);
		item->kcat = 0;
	}

	// check split category #1340142
	nbsplit = da_transaction_splits_count(item);
	for(i=0;i<nbsplit;i++)
	{
	Split *split = item->splits[i];
		cat = da_cat_get(split->kcat);
		if(cat == NULL)
		{
			g_warning("txn consistency: fixed invalid split cat %d", split->kcat);
			split->kcat = 0;
		}
	}
	
	// check payee exists
	pay = da_pay_get(item->kpay);
	if(pay == NULL)
	{
		g_warning("txn consistency: fixed invalid pay %d", item->kpay);
		item->kpay = 0;
	}

	// reset dst acc for non xfer transaction
	if( item->paymode != PAYMODE_INTXFER )
		item->kxferacc = 0;

	//#1295877 ensure income flag is correctly set
	item->flags &= ~(OF_INCOME);
	if( item->amount > 0)
		item->flags |= (OF_INCOME);

	//#1308745 ensure remind flag unset if reconciled
	if( item->flags & OF_VALID )
		item->flags &= ~(OF_REMIND);

}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
/* new transfer functions */

Transaction *transaction_strong_get_child_transfer(Transaction *src)
{
GList *list;

	DB( g_print("\n[transaction] transaction_strong_get_child_transfer\n") );

	DB( g_print(" - search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->kacc, src->kxferacc) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		Transaction *item = list->data;
		//#1252230
		//if( item->paymode == PAYMODE_INTXFER && item->kacc == src->kxferacc && item->kxfer == src->kxfer )
		if( item->paymode == PAYMODE_INTXFER && item->kxfer == src->kxfer && item != src )
		{
			DB( g_print(" - found : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->kacc, item->kxferacc) );
			return item;
		}
		list = g_list_next(list);
	}
	DB( g_print(" - not found...\n") );
	return NULL;
}


/*
 * this function retrieve into a glist the potential child transfer
 * for the source transaction
 */
GList *transaction_match_get_child_transfer(Transaction *src)
{
GList *list;
GList *match = NULL;

	DB( g_print("\n[transaction] transaction_match_get_child_transfer\n") );

	//DB( g_print(" - search : %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->kxferacc) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		Transaction *item = list->data;
		if( src->date == item->date &&
		    src->kxferacc == item->kacc &&
		    ABS(src->amount) == ABS(item->amount) &&
		    item->kxfer == 0)
		{
			//DB( g_print(" - match : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->account, item->kxferacc) );

			match = g_list_append(match, item);
		}
		list = g_list_next(list);
	}

	DB( g_print(" - found : %d\n", g_list_length(match)) );

	return match;
}


void transaction_xfer_search_or_add_child(Transaction *ope, GtkWidget *treeview)
{
GList *matchlist = transaction_match_get_child_transfer(ope);

guint count = g_list_length(matchlist);


	DB( g_print("\n[transaction] transaction_xfer_search_or_add_child\n") );

	DB( g_print(" - found result is %d, switching\n", count) );

	switch(count)
	{
		case 0:		//we should create the child
			transaction_xfer_create_child(ope, treeview);
			break;

		//todo: maybe with just 1 match the user must choose ?
		//#942346: bad idea so to no let the user confirm, so let hil confirm
		/*
		case 1:		//transform the transaction to a child transfer
		{
			GList *list = g_list_first(matchlist);
			transaction_xfer_change_to_child(ope, list->data);
			break;
		}
		*/

		default:	//the user must choose himself
		{
		Transaction *child;

			child = ui_dialog_transaction_xfer_select_child(matchlist);
			if(child == NULL)
				transaction_xfer_create_child(ope, treeview);
			else
				transaction_xfer_change_to_child(ope, child);
			break;
		}
	}

	g_list_free(matchlist);

}






void transaction_xfer_create_child(Transaction *ope, GtkWidget *treeview)
{
Transaction *child;
Account *acc;
gchar swap;

	DB( g_print("\n[transaction] transaction_xfer_create_child\n") );

	if( ope->kxferacc > 0 )
	{
		child = da_transaction_clone(ope);

		child->amount = -child->amount;
		child->flags ^= (OF_INCOME);	// invert flag
		child->flags &= ~(OF_REMIND);	// remove flag
		//#1268026
		child->flags &= ~(OF_VALID);	// remove reconcile state
		

		swap = child->kacc;
		child->kacc = child->kxferacc;
		child->kxferacc = swap;

		/* update acc flags */
		acc = da_acc_get( child->kacc );
		if(acc != NULL)
		{
			acc->flags |= AF_ADDED;

			//strong link
			guint maxkey = da_transaction_get_max_kxfer();

			DB( g_print(" + maxkey is %d\n", maxkey) );


			ope->kxfer = maxkey+1;
			child->kxfer = maxkey+1;

			DB( g_print(" + strong link to %d\n", ope->kxfer) );


			DB( g_print(" + add transfer, %p\n", child) );

			da_transaction_insert_sorted(child);

			account_balances_add (child);

			if(treeview != NULL)
				transaction_add_treeview(child, treeview, ope->kacc);
		}
	}

}


void transaction_xfer_change_to_child(Transaction *ope, Transaction *child)
{
Account *acc;

	DB( g_print("\n[transaction] transaction_xfer_change_to_child\n") );

	child->paymode = PAYMODE_INTXFER;

	ope->kxferacc = child->kacc;
	child->kxferacc = ope->kacc;

	/* update acc flags */
	acc = da_acc_get( child->kacc);
	if(acc != NULL)
		acc->flags |= AF_CHANGED;

	//strong link
	guint maxkey = da_transaction_get_max_kxfer();
	ope->kxfer = maxkey+1;
	child->kxfer = maxkey+1;

}


void transaction_xfer_sync_child(Transaction *s_txn, Transaction *child)
{

	DB( g_print("\n[transaction] transaction_xfer_sync_child\n") );

	account_balances_sub (child);

	child->date		= s_txn->date;
	child->amount   = -s_txn->amount;
	child->flags	= child->flags | OF_CHANGED;
	//#1295877
	child->flags &= ~(OF_INCOME);
	if( child->amount > 0)
	  child->flags |= (OF_INCOME);
	child->kpay		= s_txn->kpay;
	child->kcat		= s_txn->kcat;
	if(child->wording)
		g_free(child->wording);
	child->wording	= g_strdup(s_txn->wording);
	if(child->info)
		g_free(child->info);
	child->info		= g_strdup(s_txn->info);

	//#1252230 sync account also
	child->kacc		= s_txn->kxferacc;
	child->kxferacc = s_txn->kacc;

	account_balances_add (child);
	
	//todo: synchronise tags here also ?

}


void transaction_xfer_delete_child(Transaction *src)
{
Transaction *dst;

	DB( g_print("\n[transaction] transaction_xfer_delete_child\n") );

	dst = transaction_strong_get_child_transfer( src );

	DB( g_print(" -> return is %s, %p\n", dst->wording, dst) );

	if( dst != NULL )
	{
		DB( g_print("deleting...") );
		src->kxfer = 0;
		src->kxferacc = 0;
		account_balances_sub(dst);
		GLOBALS->ope_list = g_list_remove(GLOBALS->ope_list, dst);
	}
}


Transaction *transaction_old_get_child_transfer(Transaction *src)
{
GList *list;
Transaction *item;

	DB( g_print("\n[transaction] transaction_get_child_transfer\n") );

	//DB( g_print(" search: %d %s %f %d=>%d\n", src->date, src->wording, src->amount, src->account, src->kxferacc) );

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
		item = list->data;
		if( item->paymode == PAYMODE_INTXFER)
		{
			if( src->date == item->date &&
			    src->kacc == item->kxferacc &&
			    src->kxferacc == item->kacc &&
			    ABS(src->amount) == ABS(item->amount) )
			{
				//DB( g_print(" found : %d %s %f %d=>%d\n", item->date, item->wording, item->amount, item->account, item->kxferacc) );

				return item;
			}
		}
		list = g_list_next(list);
	}

	DB( g_print(" not found...\n") );

	return NULL;
}




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


void transaction_add(Transaction *ope, GtkWidget *treeview, guint32 accnum)
{
Transaction *newope;
Account *acc;

	DB( g_print("\n[transaction] transaction add\n") );

	//controls accounts valid (archive scheduled maybe bad)
	acc = da_acc_get(ope->kacc);
	if(acc == NULL) return;

	if(ope->paymode == PAYMODE_INTXFER)
	{
		acc = da_acc_get(ope->kxferacc);
		if(acc == NULL) return;
		
		// remove any splits
		da_transaction_splits_free(ope);
	}

	//allocate a new entry and copy from our edited structure
	newope = da_transaction_clone(ope);

	//init flag and keep remind status
	// already done in deftransaction_get
	//ope->flags |= (OF_ADDED);
	//remind = (ope->flags & OF_REMIND) ? TRUE : FALSE;
	//ope->flags &= (~OF_REMIND);

	/* cheque number is already stored in deftransaction_get */
	/* todo:move this to transaction add
		store a new cheque number into account ? */

	if( (newope->paymode == PAYMODE_CHECK) && (newope->info) && !(newope->flags & OF_INCOME) )
	{
	guint cheque;

		/* get the active account and the corresponding cheque number */
		acc = da_acc_get( newope->kacc);
		cheque = atol(newope->info);

		//DB( g_print(" -> should store cheque number %d to %d", cheque, newope->account) );
		if( newope->flags & OF_CHEQ2 )
		{
			acc->cheque2 = MAX(acc->cheque2, cheque);
		}
		else
		{
			acc->cheque1 = MAX(acc->cheque1, cheque);
		}
	}

	/* add normal transaction */
	acc = da_acc_get( newope->kacc);
	if(acc != NULL)
	{
		acc->flags |= AF_ADDED;

		DB( g_print(" + add normal %p\n", newope) );
		//da_transaction_append(newope);
		da_transaction_insert_sorted(newope);

		if(treeview != NULL)
			transaction_add_treeview(newope, treeview, accnum);

		account_balances_add(newope);

		if(newope->paymode == PAYMODE_INTXFER)
		{
			transaction_xfer_search_or_add_child(newope, treeview);
		}
	}
}




void transaction_add_treeview(Transaction *ope, GtkWidget *treeview, guint32 accnum)
{
GtkTreeModel *model;
GtkTreeIter  iter;
//GtkTreePath *path;
//GtkTreeSelection *sel;

	DB( g_print("\n[transaction] transaction add treeview\n") );

	if(ope->kacc == accnum)
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);

		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				LST_DSPOPE_DATAS, ope,
				-1);

		//activate that new line
		//path = gtk_tree_model_get_path(model, &iter);
		//gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeview), path);

		//sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		//gtk_tree_selection_select_iter(sel, &iter);

		//gtk_tree_path_free(path);

	}
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static gboolean misc_text_match(gchar *text, gchar *searchtext, gboolean exact)
{
gboolean match = FALSE;

	if(text == NULL)
		return FALSE;
	
	//DB( g_print("search %s in %s\n", rul->name, ope->wording) );
	if( searchtext != NULL )
	{
		if( exact == TRUE )
		{
			if( g_strrstr(text, searchtext) != NULL )
			{
				DB( g_print(" found case '%s'\n", searchtext) );
				match = TRUE;
			}
		}
		else
		{
		gchar *word   = g_utf8_casefold(text, -1);
		gchar *needle = g_utf8_casefold(searchtext, -1);

			if( g_strrstr(word, needle) != NULL )
			{
				DB( g_print(" found nocase '%s'\n", searchtext) );
				match = TRUE;
			}
			g_free(word);
			g_free(needle);
		}
	}

	return match;
}


static Assign *transaction_auto_assign_eval_txn(GList *l_rul, Transaction *txn)
{
Assign *rule = NULL;
GList *list;
	
	DB( g_print("- eval every rules, and return the last that match\n") );

	list = g_list_first(l_rul);
	while (list != NULL)
	{
	Assign *rul = list->data;
	gchar *text;

		text = txn->wording;
		if(rul->field == 1) //payee
		{
		Payee *pay = da_pay_get(txn->kpay);
			if(pay)
				text = pay->name;
		}
		if( misc_text_match(text, rul->name, rul->flags & ASGF_EXACT))
			rule = rul;

		list = g_list_next(list);
	}

	return rule;
}


static Assign *transaction_auto_assign_eval(GList *l_rul, gchar *text)
{
Assign *rule = NULL;
GList *list;
	
	DB( g_print("- eval every rules, and return the last that match\n") );

	list = g_list_first(l_rul);
	while (list != NULL)
	{
	Assign *rul = list->data;

		if( rul->field == 0 )   //memo
		{
			if( misc_text_match(text, rul->name, rul->flags & ASGF_EXACT))
				rule = rul;
		}
		list = g_list_next(list);
	}

	return rule;
}


gint transaction_auto_assign(GList *ope_list, guint32 key)
{
GList *l_ope;
GList *l_rul;
gint changes = 0;

	DB( g_print("\n[transaction] transaction_auto_assign\n") );

	l_ope = g_list_first(ope_list);
	l_rul = g_hash_table_get_values(GLOBALS->h_rul);

	while (l_ope != NULL)
	{
	Transaction *ope = l_ope->data;

		DB( g_print("- eval ope '%s' : acc=%d, pay=%d, cat=%d\n", ope->wording, ope->kacc, ope->kpay, ope->kcat) );

		//#1215521: added key == 0
		if( (key == ope->kacc || key == 0) )
		{
		Assign *rul;

			if( !(ope->flags & OF_SPLIT) && (ope->kpay == 0 || ope->kcat == 0) )
			{
				rul = transaction_auto_assign_eval_txn(l_rul, ope);
				if( rul != NULL )
				{
					if( ope->kpay == 0 && (rul->flags & ASGF_DOPAY) )
					{
						ope->kpay = rul->kpay;
						ope->flags |= OF_CHANGED;
						changes++;
					}
					if( ope->kcat == 0 && (rul->flags & ASGF_DOCAT) )
					{
						ope->kcat = rul->kcat;
						ope->flags |= OF_CHANGED;
						changes++;
					}
					
				}
			}
			else if( ope->flags & OF_SPLIT )
			{
			guint i, nbsplit = da_transaction_splits_count(ope);
			Split *split;
			gboolean split_change = FALSE;

				for(i=0;i<nbsplit;i++)
				{
					split = ope->splits[i];

					DB( g_print("- eval split '%s'\n", split->memo) );

					if(split->kcat == 0)
					{
						rul = transaction_auto_assign_eval(l_rul, split->memo);
						if( rul != NULL )
						{
							if( split->kcat == 0 && rul->kcat > 0 )
							{
								split->kcat = rul->kcat;
								ope->flags |= OF_CHANGED;
								split_change = TRUE;
							}
						}
					}

				}

				if(split_change == TRUE)
					changes++;

			}
			
		}

		l_ope = g_list_next(l_ope);
	}

	g_list_free(l_rul);

	return changes;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint
transaction_tags_count(Transaction *ope)
{
guint count = 0;
guint32 *ptr = ope->tags;

	if( ope->tags == NULL )
		return 0;

	while(*ptr++ != 0 && count < 32)
		count++;

	return count;
}




guint transaction_splits_parse(Transaction *ope, gchar *cats, gchar *amounts, gchar *memos)
{
gchar **cat_a, **amt_a, **mem_a;
guint count, i;
guint32 kcat;
gdouble amount;
Split *split;

	DB( g_print(" split parse %s :: %s :: %s\n", cats, amounts, memos) );

	cat_a = g_strsplit (cats, "||", 0);
	amt_a = g_strsplit (amounts, "||", 0);
	mem_a = g_strsplit (memos, "||", 0);

	count = g_strv_length(amt_a);
	if( (count == g_strv_length(cat_a)) && (count == g_strv_length(mem_a)) )
	{
		for(i=0;i<count;i++)
		{
			kcat = atoi(cat_a[i]);
			amount = g_ascii_strtod(amt_a[i], NULL);
			split = da_split_new(kcat, amount, mem_a[i]);
			da_transaction_splits_append (ope, split);
		}
		
		ope->flags |= OF_SPLIT;
	}
	else
	{
		g_warning("invalid split parse");
	}

	g_strfreev (mem_a);
	g_strfreev (amt_a);
	g_strfreev (cat_a);

	return count;
}



guint transaction_splits_tostring(Transaction *ope, gchar **cats, gchar **amounts, gchar **memos)
{
guint count, i;
Split *split;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
GString *cat_a = g_string_new (NULL);
GString *amt_a = g_string_new (NULL);
GString *mem_a = g_string_new (NULL);

	count = da_transaction_splits_count(ope);
	for(i=0;i<count;i++)
	{
		split = ope->splits[i];
		g_string_append_printf (cat_a, "%d", split->kcat);
		g_string_append(amt_a, g_ascii_dtostr (buf, sizeof (buf), split->amount) );
		g_string_append(mem_a, split->memo);

		if((i+1) < count)
		{
			g_string_append(cat_a, "||");
			g_string_append(amt_a, "||");
			g_string_append(mem_a, "||");
		}		
	}	

	*cats = g_string_free(cat_a, FALSE);
	*amounts = g_string_free(amt_a, FALSE);
	*memos = g_string_free(mem_a, FALSE);
	
	return count;
}


guint
transaction_tags_parse(Transaction *ope, const gchar *tagstring)
{
gchar **str_array;
guint count, i;
Tag *tag;

	DB( g_print("(transaction_set_tags)\n") );

	DB( g_print(" - tagstring='%s'\n", tagstring) );

	str_array = g_strsplit (tagstring, " ", 0);
	count = g_strv_length( str_array );

	g_free(ope->tags);
	ope->tags = NULL;

	DB( g_print(" -> reset storage %p\n", ope->tags) );


	if( count > 0 )
	{

		ope->tags = g_new0(guint32, count + 1);

		DB( g_print(" -> storage %p\n", ope->tags) );

		for(i=0;i<count;i++)
		{
			tag = da_tag_get_by_name(str_array[i]);
			if(tag == NULL)
			{
			Tag *newtag = da_tag_malloc();

				newtag->name = g_strdup(str_array[i]);
				da_tag_append(newtag);
				tag = da_tag_get_by_name(str_array[i]);
			}

			DB( g_print(" -> storing %d=>%s at tags pos %d\n", tag->key, tag->name, i) );

			ope->tags[i] = tag->key;
		}
	}

	//hex_dump(ope->tags, sizeof(guint32*)*count+1);

	g_strfreev (str_array);

	return count;
}

gchar *
transaction_tags_tostring(Transaction *ope)
{
guint count, i;
gchar **str_array;
gchar *tagstring;
Tag *tag;

	DB( g_print("transaction_get_tagstring\n") );

	DB( g_print(" -> tags at=%p\n", ope->tags) );

	if( ope->tags == NULL )
	{

		return NULL;
	}
	else
	{
		count = transaction_tags_count(ope);

		DB( g_print(" -> tags at=%p, nbtags=%d\n", ope->tags, count) );

		str_array = g_new0(gchar*, count+1);

		DB( g_print(" -> str_array at %p\n", str_array) );

		//hex_dump(ope->tags, sizeof(guint32*)*(count+1));

		for(i=0;i<count;i++)
		{
			DB( g_print(" -> try to get tag %d\n", ope->tags[i]) );

			tag = da_tag_get(ope->tags[i]);
			if( tag )
			{
				DB( g_print(" -> get %s at %d\n", tag->name, i) );
				str_array[i] = tag->name;
			}
			else
				str_array[i] = NULL;


		}

		tagstring = g_strjoinv(" ", str_array);

		g_free (str_array);

	}

	return tagstring;
}

