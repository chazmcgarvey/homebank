/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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
#include "hb-payee.h"


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

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void
da_pay_free(Payee *item)
{
	DB( g_print("da_pay_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item);
	}
}


Payee *
da_pay_malloc(void)
{
	DB( g_print("da_pay_malloc\n") );
	return g_malloc0(sizeof(Payee));
}


void
da_pay_destroy(void)
{
	DB( g_print("da_pay_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_pay);
}


void
da_pay_new(void)
{
Payee *item;

	DB( g_print("da_pay_new\n") );
	GLOBALS->h_pay = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_pay_free);

	// insert our 'no payee'
	item = da_pay_malloc();
	item->name = g_strdup("");
	da_pay_insert(item);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_pay_max_key_ghfunc(gpointer key, Payee *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_pay_name_grfunc(gpointer key, Payee *item, gchar *name)
{
	if( name && item->name )
	{
		if(!strcasecmp(name, item->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_pay_length:
 *
 * Return value: the number of elements
 */
guint
da_pay_length(void)
{
	return g_hash_table_size(GLOBALS->h_pay);
}

/*
gboolean
da_pay_create_none(void)
{
Payee *pay;
guint32 *new_key;

	DB( g_print("da_pay_insert none\n") );

	pay = da_pay_malloc();
	new_key = g_new0(guint32, 1);
	*new_key = 0;
	pay->key = 0;
	pay->name = g_strdup("");

	DB( g_print(" -> insert id: %d\n", *new_key) );

	g_hash_table_insert(GLOBALS->h_pay, new_key, pay);


	return TRUE;
}
*/


/**
 * da_pay_remove:
 *
 * delete an payee from the GHashTable
 *
 * Return value: TRUE if the key was found and deleted
 *
 */
gboolean
da_pay_remove(guint32 key)
{
	DB( g_print("da_pay_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_pay, &key);
}

/**
 * da_pay_insert:
 *
 * insert an payee into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_pay_insert(Payee *item)
{
guint32 *new_key;

	DB( g_print("da_pay_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_pay, new_key, item);

	return TRUE;
}


/**
 * da_pay_append:
 *
 * append a new payee into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_pay_append(Payee *item)
{
Payee *existitem;
guint32 *new_key;

	DB( g_print("da_pay_append\n") );

	/* ensure no duplicate */
	//g_strstrip(item->name);
	if( item->name != NULL )
	{
		existitem = da_pay_get_by_name( item->name );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_pay_get_max_key() + 1;
			item->key = *new_key;

			DB( g_print(" -> append id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_pay, new_key, item);
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->name, item->key) );

	return FALSE;
}

/**
 * da_pay_get_max_key:
 *
 * Get the biggest key from the GHashTable
 *
 * Return value: the biggest key value
 *
 */
guint32
da_pay_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_pay, (GHFunc)da_pay_max_key_ghfunc, &max_key);
	return max_key;
}




/**
 * da_pay_get_by_name:
 *
 * Get an payee structure by its name
 *
 * Return value: Payee * or NULL if not found
 *
 */
Payee *
da_pay_get_by_name(gchar *name)
{
	DB( g_print("da_pay_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_pay, (GHRFunc)da_pay_name_grfunc, name);
}



/**
 * da_pay_get:
 *
 * Get an payee structure by key
 *
 * Return value: Payee * or NULL if not found
 *
 */
Payee *
da_pay_get(guint32 key)
{
	//DB( g_print("da_pay_get\n") );

	return g_hash_table_lookup(GLOBALS->h_pay, &key);
}


void da_pay_consistency(Payee *item)
{
	g_strstrip(item->name);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#if MYDEBUG

static void
da_pay_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Payee *item = value;

	DB( g_print(" %d :: %s\n", *id, item->name) );

}

static void
da_pay_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_pay, da_pay_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void 
payee_delete_unused(void)
{
GList *lpay, *list;
	
	lpay = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *entry = list->data;

		if(entry->usage_count <= 0 && entry->key > 0)
			da_pay_remove (entry->key);
		list = g_list_next(list);
	}
	g_list_free(lpay);
}


void 
payee_fill_usage(void)
{
GList *lpay;
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GList *lrul, *list;

	lpay = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *entry = list->data;
		entry->usage_count = 0;
		list = g_list_next(list);
	}
	g_list_free(lpay);


	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;
		Payee *pay = da_pay_get (txn->kpay);
			
			if(pay)
				pay->usage_count++;

			lnk_txn = g_list_next(lnk_txn);
		}

		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
	Payee *pay = da_pay_get (entry->kpay);
		
		if(pay)
			pay->usage_count++;
		list = g_list_next(list);
	}

	lrul = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;
	Payee *pay = da_pay_get (entry->kpay);

		if(pay)
			pay->usage_count++;

		list = g_list_next(list);
	}
	g_list_free(lrul);

}


void
payee_move(guint32 key1, guint32 key2)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GList *lrul, *list;

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;
	
			if(txn->kpay == key1)
			{
				txn->kpay = key2;
				txn->flags |= OF_CHANGED;
			}
			lnk_txn = g_list_next(lnk_txn);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if(entry->kpay == key1)
		{
			entry->kpay = key2;
		}
		list = g_list_next(list);
	}

	lrul = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if(entry->kpay == key1)
		{
			entry->kpay = key2;
		}
		list = g_list_next(list);
	}
	g_list_free(lrul);
}


gboolean
payee_rename(Payee *item, const gchar *newname)
{
Payee *existitem;
gchar *stripname;

	stripname = g_strdup(newname);
	g_strstrip(stripname);

	existitem = da_pay_get_by_name(stripname);

	if( existitem != NULL )
	{
		if( existitem->key == item->key )
			return TRUE;
	}
	else
	{
		g_free(item->name);
		item->name = g_strdup(stripname);
		return TRUE;
	}

	g_free(stripname);

	return FALSE;
}


/**
 * payee_append_if_new:
 *
 * append a new payee into the GHashTable
 *
 * Return value: a new Payee or NULL
 *
 */
Payee *
payee_append_if_new(gchar *name)
{
gchar *stripname;
Payee *item;

	stripname = g_strdup(name);
	g_strstrip(stripname);
	item = da_pay_get_by_name(stripname);

	if(item == NULL)
	{
		item = da_pay_malloc();
		item->name = g_strdup(stripname);
		da_pay_append(item);
	}
	else
		item = NULL;

	g_free(stripname);

	return item;
}

static gint
payee_glist_name_compare_func(Payee *a, Payee *b)
{
	return hb_string_utf8_compare(a->name, b->name);
}


static gint
payee_glist_key_compare_func(Payee *a, Payee *b)
{
	return a->key - b->key;
}


GList *payee_glist_sorted(gint column)
{
GList *list = g_hash_table_get_values(GLOBALS->h_pay);

	if(column == 0)
		return g_list_sort(list, (GCompareFunc)payee_glist_key_compare_func);
	else
		return g_list_sort(list, (GCompareFunc)payee_glist_name_compare_func);
}



void
payee_load_csv(gchar *filename)
{
GIOChannel *io;
gchar *tmpstr;
gint io_stat;
const gchar *encoding;

	encoding = homebank_file_getencoding(filename);

	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{
		DB( g_print(" -> encoding should be %s\n", encoding) );
		if( encoding != NULL )
		{
			g_io_channel_set_encoding(io, encoding, NULL);
		}

		for(;;)
		{
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, NULL);
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != NULL)
				{
					hb_string_strip_crlf(tmpstr);

					DB( g_print(" read %s\n", tmpstr) );

					if( payee_append_if_new( tmpstr ) )
					{
						GLOBALS->changes_count++;
					}

				}
				g_free(tmpstr);
			}

		}
		g_io_channel_unref (io);
	}

}


void
payee_save_csv(gchar *filename)
{
GIOChannel *io;
GList *lpay, *list;
gchar *outstr;

	io = g_io_channel_new_file(filename, "w", NULL);
	if(io != NULL)
	{
		lpay = list = payee_glist_sorted(1);

		while (list != NULL)
		{
		Payee *item = list->data;

			if(item->key != 0)
			{
				outstr = g_strdup_printf("%s\n", item->name);
				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				DB( g_print("%s", outstr) );

				g_free(outstr);
			}
			list = g_list_next(list);
		}
		g_list_free(lpay);

		g_io_channel_unref (io);
	}

}


