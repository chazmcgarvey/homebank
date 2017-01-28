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

#include "hb-transaction.h"
#include "hb-split.h"

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



static Split *da_split_record_clone(Split *src_split)
{
Split *new_split = g_memdup(src_split, sizeof(Split));

	DB( g_print("da_split_record_clone\n") );

	if(new_split)
	{
		//duplicate the string
		new_split->memo = g_strdup(src_split->memo);
		DB( g_print(" clone %p -> %p\n", src_split, new_split) );

	}
	return new_split;
}

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint da_splits_count(Split *txn_splits[])
{
guint i, count = 0;

	for(i=0;i<TXN_MAX_SPLIT;i++)
	{
		if(txn_splits[i] == NULL)
			break;
		count++;
	}
	return count;
}


void da_splits_free(Split *txn_splits[])
{
guint count, i=0;

	count = da_splits_count(txn_splits);
	if(count == 0)
		return;
	
	DB( g_print("da_splits_free\n") );

	for(;i<=count;i++)
	{
		DB( g_print("- freeing %d :: %p\n", i, txn_splits[i]) );
		
		da_split_free(txn_splits[i]);
		txn_splits[i] = NULL;
	}
}


void da_splits_append(Split *txn_splits[], Split *new_split)
{
guint count = da_splits_count(txn_splits);

	DB( g_print("da_splits_append\n") );

	DB( g_print("- split[%d] at %p for ope \n", count, new_split) );

	txn_splits[count] = new_split;
	txn_splits[count + 1] = NULL;
	
	DB( g_print("- %d splits\n", da_splits_count(txn_splits)) );
}


guint da_splits_clone(Split *stxn_splits[], Split *dtxn_splits[])
{
gint i, count;

	DB( g_print("da_splits_clone\n") );
	
	count = da_splits_count(stxn_splits);
	for(i=0;i<count;i++)
	{
		dtxn_splits[i] = da_split_record_clone(stxn_splits[i]);
	}	

/*	if(count > 0)
		dtxn->flags |= OF_SPLIT;*/
	
	DB( g_print(" clone %p -> %p, %d splits\n", stxn_splits, dtxn_splits, count) );
	return count;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

guint da_splits_parse(Split *ope_splits[], gchar *cats, gchar *amounts, gchar *memos)
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
			da_splits_append (ope_splits, split);
		}
		
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



guint da_splits_tostring(Split *ope_splits[], gchar **cats, gchar **amounts, gchar **memos)
{
guint count, i;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
GString *cat_a = g_string_new (NULL);
GString *amt_a = g_string_new (NULL);
GString *mem_a = g_string_new (NULL);

	count = da_splits_count(ope_splits);
	for(i=0;i<count;i++)
	{
		g_string_append_printf (cat_a, "%d", ope_splits[i]->kcat);
		g_string_append(amt_a, g_ascii_dtostr (buf, sizeof (buf), ope_splits[i]->amount) );
		g_string_append(mem_a, ope_splits[i]->memo);

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

void split_cat_consistency (Split *txn_splits[])
{
	guint i, nbsplit;
	
	// check split category #1340142
	nbsplit = da_splits_count(txn_splits);
	for(i=0;i<nbsplit;i++)
	{
		if(da_cat_get(txn_splits[i]->kcat) == NULL)
		{
			g_warning("split consistency: fixed invalid split cat %d", txn_splits[i]->kcat);
			txn_splits[i]->kcat = 0;
		}
	}
}

