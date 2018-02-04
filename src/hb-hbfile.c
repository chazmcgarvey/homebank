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
#include "hb-hbfile.h"
#include "hb-archive.h"
#include "hb-transaction.h"


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


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


gboolean hbfile_file_isbackup(gchar *filepath)
{
	return g_str_has_suffix(filepath, "xhb~");
}





gboolean hbfile_file_hasbackup(gchar *filepath)
{
gchar *bakfilepath;

	bakfilepath = hb_filename_new_with_extension(GLOBALS->xhb_filepath, "xhb~");
	GLOBALS->xhb_hasbak = g_file_test(bakfilepath, G_FILE_TEST_EXISTS);
	g_free(bakfilepath);
	//todo check here if need to return something
	return GLOBALS->xhb_hasbak;
}


void hbfile_file_default(void)
{
	//todo: maybe translate this also
	hbfile_change_filepath(g_build_filename(PREFS->path_hbfile, "untitled.xhb", NULL));
	GLOBALS->hbfile_is_new = TRUE;
	GLOBALS->hbfile_is_bak = FALSE;
	
	DB( g_print("- path_hbfile is '%s'\n", PREFS->path_hbfile) );
	DB( g_print("- xhb_filepath is '%s'\n", GLOBALS->xhb_filepath) );
}



/*
static gint hbfile_file_load_xhb(gchar *filepath)
{






}


static void hbfile_file_load_backup_xhb(void)
{
//todo: get from dialog.c, and split between dilaog.c/hbfile.c



}
*/

void hbfile_replace_basecurrency(Currency4217 *curfmt)
{
Currency *item;
guint32 oldkcur;

	DB( g_print("\n[hbfile] replace base currency \n") );
	
	oldkcur = GLOBALS->kcur;
	da_cur_remove(oldkcur);
	item = currency_add_from_user(curfmt);
	GLOBALS->kcur = item->key;
	
	DB( g_print(" %d ==> %d %s\n", oldkcur, GLOBALS->kcur, item->iso_code) );
}


void hbfile_change_basecurrency(guint32 key)
{
GList *list;
guint32 oldkcur;
	
	// set every rate to 0
	list = g_hash_table_get_values(GLOBALS->h_cur);
	while (list != NULL)
	{
	Currency *entry = list->data;

		if(entry->key != GLOBALS->kcur)
		{
			entry->rate = 0.0;
			entry->mdate = 0;
		}
			
		list = g_list_next(list);
	}
	g_list_free(list);	

	oldkcur = GLOBALS->kcur;
	GLOBALS->kcur = key;

	// update account with old base currency
	list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *acc = list->data;

		if( acc->kcur == oldkcur )
			acc->kcur = key;

		list = g_list_next(list);
	}
	g_list_free(list);
	

	GLOBALS->changes_count++;
}


GList *hbfile_transaction_get_all(void)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GList *list;

	list = NULL;

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		//#1674045 ony rely on nosummary
		//if( (acc->flags & (AF_CLOSED|AF_NOREPORT)) )
		if( (acc->flags & (AF_NOREPORT)) )
			goto next_acc;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
			list = g_list_append(list, lnk_txn->data);
			lnk_txn = g_list_next(lnk_txn);
		}
	
	next_acc:
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	return da_transaction_sort (list);
}


static GQueue *hbfile_transaction_get_partial_internal(guint32 minjulian, guint32 maxjulian, gushort exclusionflags)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GQueue *txn_queue;

	txn_queue = g_queue_new ();

	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		if( (acc->flags & exclusionflags) )
			goto next_acc;

		lnk_txn = g_queue_peek_tail_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;

			if( txn->date < minjulian ) //no need to go below mindate
				break;

			if( !(txn->status == TXN_STATUS_REMIND) 
				&& (txn->date >= minjulian) 
				&& (txn->date <= maxjulian)
			  )
			{
				g_queue_push_head (txn_queue, txn);
			}
			
			lnk_txn = g_list_previous(lnk_txn);
		}
	
	next_acc:
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

	return txn_queue;
}


GQueue *hbfile_transaction_get_partial(guint32 minjulian, guint32 maxjulian)
{
	//#1674045 ony rely on nosummary
	//return hbfile_transaction_get_partial_internal(minjulian, maxjulian, (AF_CLOSED|AF_NOREPORT));
	return hbfile_transaction_get_partial_internal(minjulian, maxjulian, (AF_NOREPORT));
}


GQueue *hbfile_transaction_get_partial_budget(guint32 minjulian, guint32 maxjulian)
{
	//#1674045 ony rely on nosummary
	//return hbfile_transaction_get_partial_internal(minjulian, maxjulian, (AF_CLOSED|AF_NOREPORT|AF_NOBUDGET));
	return hbfile_transaction_get_partial_internal(minjulian, maxjulian, (AF_NOREPORT|AF_NOBUDGET));
}


void hbfile_sanity_check(void)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GList *lxxx, *list;

	DB( g_print("\n[hbfile] !! sanity_check !! \n") );

	DB( g_print(" - transaction\n") );
	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *txn = lnk_txn->data;

			da_transaction_consistency(txn);
			lnk_txn = g_list_next(lnk_txn);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);


	DB( g_print(" - scheduled/template\n") );
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;

		da_archive_consistency(entry);
		list = g_list_next(list);
	}


	DB( g_print(" - account\n") );
	lxxx = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;

		da_acc_consistency(item);
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	
	DB( g_print(" - payee\n") );
	lxxx = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		da_pay_consistency(item);
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	
	DB( g_print(" - category\n") );
	lxxx = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		da_cat_consistency(item);
		list = g_list_next(list);
	}
	g_list_free(lxxx);
	
}


void hbfile_anonymize(void)
{
GList *lst_acc, *lnk_acc;
GList *lnk_txn;
GList *lxxx, *list;
guint cnt, i;

	DB( g_print("\n[hbfile] anonymize\n") );

	// owner
	hbfile_change_owner(g_strdup("An0nym0us"));
	GLOBALS->changes_count++;
	GLOBALS->hbfile_is_new = TRUE;

	// filename
	hbfile_change_filepath(g_build_filename(PREFS->path_hbfile, "anonymized.xhb", NULL));

	// accounts
	lxxx = list = g_hash_table_get_values(GLOBALS->h_acc);
	while (list != NULL)
	{
	Account *item = list->data;
		g_free(item->name);
		item->name = g_strdup_printf("account %d", item->key);
		g_free(item->number);
		item->number = NULL;
		g_free(item->bankname);
		item->bankname = NULL;
		
		GLOBALS->changes_count++;
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	//payees
	lxxx = list = g_hash_table_get_values(GLOBALS->h_pay);
	while (list != NULL)
	{
	Payee *item = list->data;

		if(item->key != 0)
		{
			g_free(item->name);
			item->name = g_strdup_printf("payee %d", item->key);
			GLOBALS->changes_count++;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	//categories
	lxxx = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *item = list->data;

		if(item->key != 0)
		{
			g_free(item->name);
			item->name = g_strdup_printf("category %d", item->key);
			GLOBALS->changes_count++;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	//tags
	lxxx = list = g_hash_table_get_values(GLOBALS->h_tag);
	while (list != NULL)
	{
	Tag *item = list->data;

		if(item->key != 0)
		{
			g_free(item->name);
			item->name = g_strdup_printf("tag %d", item->key);
			GLOBALS->changes_count++;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	//assigns
	lxxx = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *item = list->data;

		if(item->key != 0)
		{
			g_free(item->text);
			item->text = g_strdup_printf("assign %d", item->key);
			GLOBALS->changes_count++;
		}
		list = g_list_next(list);
	}
	g_list_free(lxxx);

	//archives
	cnt = 0;
	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *item = list->data;
	
		g_free(item->memo);
		item->memo = g_strdup_printf("archive %d", cnt++);
		GLOBALS->changes_count++;
		
		//later split anonymize also
		
		list = g_list_next(list);
	}

	//transaction
	lst_acc = g_hash_table_get_values(GLOBALS->h_acc);
	lnk_acc = g_list_first(lst_acc);
	while (lnk_acc != NULL)
	{
	Account *acc = lnk_acc->data;

		lnk_txn = g_queue_peek_head_link(acc->txn_queue);
		while (lnk_txn != NULL)
		{
		Transaction *item = lnk_txn->data;
		Split *split;

			g_free(item->info);
			item->info = NULL;
			g_free(item->memo);
			item->memo = g_strdup_printf("memo %d", item->date);
			GLOBALS->changes_count++;
		
			if(item->flags & OF_SPLIT)
			{
				for(i=0;i<TXN_MAX_SPLIT;i++)
				{
					split = item->splits[i];
					if( split == NULL ) break;

					if(split->memo != NULL)
						g_free(split->memo);
					
					split->memo = g_strdup_printf("memo %d", i);
					GLOBALS->changes_count++;
				}		
			}
			lnk_txn = g_list_next(lnk_txn);
		}
		lnk_acc = g_list_next(lnk_acc);
	}
	g_list_free(lst_acc);

}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


void hbfile_change_owner(gchar *owner)
{
	g_free(GLOBALS->owner);
	GLOBALS->owner = (owner != NULL) ? owner : NULL;
}


void hbfile_change_filepath(gchar *filepath)
{
	g_free(GLOBALS->xhb_filepath);
	GLOBALS->xhb_filepath = (filepath != NULL) ? filepath : NULL;
}


void hbfile_cleanup(gboolean file_clear)
{
Transaction *txn;

	DB( g_print("\n[hbfile] cleanup\n") );
	DB( g_print("- file clear is %d\n", file_clear) );
	
	// Free data storage
	txn = g_trash_stack_pop(&GLOBALS->txn_stk);
	while( txn != NULL )
	{
		da_transaction_free (txn);
		txn = g_trash_stack_pop(&GLOBALS->txn_stk);
	}

	da_transaction_destroy();
	da_archive_destroy(GLOBALS->arc_list);
	g_hash_table_destroy(GLOBALS->h_memo);
	da_asg_destroy();
	da_tag_destroy();
	da_cat_destroy();
	da_pay_destroy();
	da_acc_destroy();
	da_cur_destroy();

	hbfile_change_owner(NULL);

	if(file_clear)
		hbfile_change_filepath(NULL);

}


void hbfile_setup(gboolean file_clear)
{

	DB( g_print("\n[hbfile] setup\n") );
	DB( g_print("- file clear is %d\n", file_clear) );

	// Allocate data storage
	da_cur_new();
	da_acc_new();
	da_pay_new();
	da_cat_new();
	da_tag_new();
	da_asg_new();

	GLOBALS->h_memo = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)g_free, NULL);
	GLOBALS->arc_list = NULL;
	GLOBALS->txn_stk = NULL;

	if(file_clear == TRUE)
	{
		//todo: maybe translate this also
		hbfile_change_filepath(g_build_filename(PREFS->path_hbfile, "untitled.xhb", NULL));
		GLOBALS->hbfile_is_new = TRUE;
		
		DB( g_print("- path_hbfile is '%s'\n", PREFS->path_hbfile) );
		DB( g_print("- xhb_filepath is '%s'\n", GLOBALS->xhb_filepath) );
	}
	else
	{
		GLOBALS->hbfile_is_new = FALSE;
	}

	hbfile_change_owner(g_strdup(_("Unknown")));
	
	GLOBALS->kcur = 1;

	GLOBALS->vehicle_category = 0;
	
	GLOBALS->auto_smode = 1;
	GLOBALS->auto_nbdays = 0;
	GLOBALS->auto_weekday = 1;
	
	GLOBALS->changes_count = 0;

	GLOBALS->xhb_hasbak = FALSE;

}

