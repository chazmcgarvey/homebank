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
#include "hb-account.h"

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


void
da_acc_free(Account *item)
{
	DB( g_print("da_acc_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->imp_name);
		g_free(item->name);
		g_free(item->number);
		g_free(item->bankname);
		g_free(item->notes);
		
		g_queue_free (item->txn_queue);
		
		g_free(item);
	}
}


Account *
da_acc_malloc(void)
{
Account *item;

	DB( g_print("da_acc_malloc\n") );
	item = g_malloc0(sizeof(Account));
	item->txn_queue = g_queue_new ();
	return item;
}


void
da_acc_destroy(void)
{
	DB( g_print("da_acc_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_acc);
}


void
da_acc_new(void)
{
	DB( g_print("da_acc_new\n") );
	GLOBALS->h_acc = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_acc_free);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_acc_max_key_ghfunc(gpointer key, Account *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_acc_name_grfunc(gpointer key, Account *item, gchar *name)
{
	if( name && item->name )
	{
		if(!strcasecmp(name, item->name))
			return TRUE;
	}
	return FALSE;
}

static gboolean da_acc_imp_name_grfunc(gpointer key, Account *item, gchar *name)
{
	if( name && item->imp_name )
	{
		if(!strcasecmp(name, item->imp_name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_acc_length:
 *
 * Return value: the number of elements
 */
guint
da_acc_length(void)
{
	return g_hash_table_size(GLOBALS->h_acc);
}


/**
 * da_acc_remove:
 *
 * delete an account from the GHashTable
 *
 * Return value: TRUE if the key was found and deleted
 *
 */
gboolean
da_acc_remove(guint32 key)
{
	DB( g_print("da_acc_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_acc, &key);
}

/**
 * da_acc_insert:
 *
 * insert an account into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_acc_insert(Account *item)
{
guint32 *new_key;

	DB( g_print("da_acc_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_acc, new_key, item);

	return TRUE;
}


/**
 * da_acc_append:
 *
 * insert an account into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_acc_append(Account *item)
{
Account *existitem;
guint32 *new_key;

	DB( g_print("da_acc_append\n") );

	/* ensure no duplicate */
	g_strstrip(item->name);
	if(item->name != NULL)
	{
		existitem = da_acc_get_by_name( item->name );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_acc_get_max_key() + 1;
			item->key = *new_key;
			item->pos = da_acc_length() + 1;

			DB( g_print(" -> insert id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_acc, new_key, item);
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->name, item->key) );

	return FALSE;
}

/**
 * da_acc_get_max_key:
 *
 * Get the biggest key from the GHashTable
 *
 * Return value: the biggest key value
 *
 */
guint32
da_acc_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_acc, (GHFunc)da_acc_max_key_ghfunc, &max_key);
	return max_key;
}




/**
 * da_acc_get_by_name:
 *
 * Get an account structure by its name
 *
 * Return value: Account * or NULL if not found
 *
 */
Account *
da_acc_get_by_name(gchar *name)
{
	DB( g_print("da_acc_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_acc, (GHRFunc)da_acc_name_grfunc, name);
}

Account *
da_acc_get_by_imp_name(gchar *name)
{
	DB( g_print("da_acc_get_by_imp_name\n") );

	return g_hash_table_find(GLOBALS->h_acc, (GHRFunc)da_acc_imp_name_grfunc, name);
}


/**
 * da_acc_get:
 *
 * Get an account structure by key
 *
 * Return value: Account * or NULL if not found
 *
 */
Account *
da_acc_get(guint32 key)
{
	//DB( g_print("da_acc_get\n") );

	return g_hash_table_lookup(GLOBALS->h_acc, &key);
}


void da_acc_consistency(Account *item)
{
	g_strstrip(item->name);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#if MYDEBUG

static void
da_acc_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Account *item = value;

	DB( g_print(" %d :: %s\n", *id, item->name) );

}

static void
da_acc_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_acc, da_acc_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif


static gint
account_glist_name_compare_func(Account *a, Account *b)
{
	return hb_string_utf8_compare(a->name, b->name);
}


static gint
account_glist_key_compare_func(Account *a, Account *b)
{
	return a->key - b->key;
}


GList *account_glist_sorted(gint column)
{
GList *list = g_hash_table_get_values(GLOBALS->h_acc);

	if(column == 0)
		return g_list_sort(list, (GCompareFunc)account_glist_key_compare_func);
	else
		return g_list_sort(list, (GCompareFunc)account_glist_name_compare_func);
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */




/**
 * account_is_used:
 *
 * controls if an account is used by any archive or transaction
 *
 * Return value: TRUE if used, FALSE, otherwise
 */
gboolean
account_is_used(guint32 key)
{
Account *acc;
GList *list;
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
gboolean retval;

	retval = FALSE;
	lst_acc = NULL;

	acc = da_acc_get(key);
	if( g_queue_get_length(acc->txn_queue) > 0 )
	{
		retval = TRUE;
		goto end;
	}

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		if(acc->key != key)
		{
			lnk_txn = g_queue_peek_head_link(acc->txn_queue);
			while (lnk_txn != NULL)
			{
			Transaction *entry = lnk_txn->data;
			
				if( key == entry->kxferacc)
				{
					retval = TRUE;
					goto end;
				}

				lnk_txn = g_list_next(lnk_txn);
			}
		}
		lnk_acc = g_list_next(lnk_acc);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;

		if( key == entry->kacc || key == entry->kxferacc)
		{
			retval = TRUE;
			goto end;
		}

		list = g_list_next(list);
	}

end:
	g_list_free(lst_acc);

	return retval;
}


static gchar *
account_get_stripname(gchar *name)
{
gchar *stripname = g_strdup(name);
	g_strstrip(stripname);

	return stripname;
}


gboolean
account_exists(gchar *name)
{
Account *existitem;
gchar *stripname = account_get_stripname(name);

	existitem = da_acc_get_by_name(stripname);
	g_free(stripname);

	return existitem == NULL ? FALSE : TRUE;
}


gboolean
account_rename(Account *item, gchar *newname)
{
Account *existitem;
gchar *stripname = account_get_stripname(newname);

	existitem = da_acc_get_by_name(stripname);
	if( existitem == NULL )
	{
		g_free(item->name);
		item->name = g_strdup(stripname);
		return TRUE;
	}

	g_free(stripname);

	return FALSE;
}


/* 
 * change the account currency
 * change every txn to currency
 * ensure dst xfer transaction account will be set to same currency
 */
void account_set_currency(Account *acc, guint32 kcur)
{
GList *list;
Account *dstacc;
gboolean *xfer_list;
guint32 maxkey, i;

	DB( g_print("\n[account] set currency\n") );

	if(acc->kcur == kcur)
	{
		DB( g_print(" - already ok, return\n") );
		return;
	}

	DB( g_print(" - set for '%s'\n", acc->name)  );
		
	maxkey = da_acc_get_max_key () + 1;
	xfer_list = g_malloc0(sizeof(gboolean) * maxkey );
	DB( g_print(" - alloc for %d account\n", da_acc_length() ) );

	list = g_queue_peek_head_link(acc->txn_queue);
	while (list != NULL)
	{
	Transaction *txn = list->data;

		txn->kcur = kcur;
		if( (txn->paymode == PAYMODE_INTXFER) && (txn->kxferacc > 0) && (txn->kxfer > 0) )
		{
			xfer_list[txn->kxferacc] = TRUE;
		}
		list = g_list_next(list);
	}

	acc->kcur = kcur;
	DB( g_print(" - '%s'\n", acc->name) );
	
	for(i=1;i<maxkey;i++)
	{
		DB( g_print(" - %d '%d'\n", i, xfer_list[i]) );
		if( xfer_list[i] == TRUE )
		{
			dstacc = da_acc_get(i);
			account_set_currency(dstacc, kcur);
		}
	}

	g_free(xfer_list);

}


/**
 * private function to sub transaction amount from account balances
 */
static void account_balances_sub_internal(Account *acc, Transaction *trn)
{
	acc->bal_future -= trn->amount;

	if(trn->date <= GLOBALS->today)
		acc->bal_today -= trn->amount;

	if(trn->status == TXN_STATUS_RECONCILED)
	//if(trn->flags & OF_VALID)
		acc->bal_bank -= trn->amount;
}

/**
 * private function to add transaction amount from account balances
 */
static void account_balances_add_internal(Account *acc, Transaction *trn)
{
	acc->bal_future += trn->amount;

	if(trn->date <= GLOBALS->today)
		acc->bal_today += trn->amount;

	if(trn->status == TXN_STATUS_RECONCILED)
	//if(trn->flags & OF_VALID)
		acc->bal_bank += trn->amount;
}


/**
 * public function to sub transaction amount from account balances
 */
gboolean account_balances_sub(Transaction *trn)
{

	if(!(trn->status == TXN_STATUS_REMIND))
	//if(!(trn->flags & OF_REMIND))
	{
		Account *acc = da_acc_get(trn->kacc);
		if(acc == NULL) return FALSE;
		account_balances_sub_internal(acc, trn);
		return TRUE;
	}
	return FALSE;
}


/**
 * public function to add transaction amount from account balances
 */
gboolean account_balances_add(Transaction *trn)
{
	if(!(trn->status == TXN_STATUS_REMIND))
	//if(!(trn->flags & OF_REMIND))
	{
		Account *acc = da_acc_get(trn->kacc);
		if(acc == NULL) return FALSE;
		account_balances_add_internal(acc, trn);
		return TRUE;
	}
	return FALSE;
}


//todo: optim called 2 times from dsp_mainwindow
void account_compute_balances(void)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;

	DB( g_print("\naccount_compute_balances start\n") );

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;
	
		/* set initial amount */
		acc->bal_bank = acc->initial;
		acc->bal_today = acc->initial;
		acc->bal_future = acc->initial;
		
		/* add every txn */
		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;
		
			if(!(txn->status == TXN_STATUS_REMIND))
			{
				account_balances_add_internal(acc, txn);
			}
			lnk_txn = g_list_next(lnk_txn);
		}
		
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	DB( g_print("\naccount_compute_balances end\n") );

}


void account_convert_euro(Account *acc)
{
GList *lnk_txn;

	lnk_txn = g_queue_peek_head_link(acc->txn_queue);
	while (lnk_txn != NULL)
	{
	Transaction *txn = lnk_txn->data;
	gdouble oldamount = txn->amount;

		txn->amount = hb_amount_to_euro(oldamount);
		DB( g_print("%10.6f => %10.6f, %s\n", oldamount, txn->amount, txn->wording) );
		//todo: sync child xfer
		lnk_txn = g_list_next(lnk_txn);
	}

	acc->initial = hb_amount_to_euro(acc->initial);
	acc->warning = hb_amount_to_euro(acc->warning);
	acc->minimum = hb_amount_to_euro(acc->minimum);
}

