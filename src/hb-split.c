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


void da_split_free(Split *item)
{
	if(item != NULL)
	{
		if(item->memo != NULL)
			g_free(item->memo);

		g_free(item);
	}
}


Split *da_split_malloc(void)
{
	return g_malloc0(sizeof(Split));
}


void da_split_destroy(GPtrArray *splits)
{
	DB( g_print("da_split_destroy\n") );
	if(splits != NULL)
		g_ptr_array_free(splits, TRUE);
}


GPtrArray *da_split_new(void)
{
	DB( g_print("da_split_new\n") );
	return g_ptr_array_new_with_free_func((GDestroyNotify)da_split_free);
}


static GPtrArray *da_split_new_full(guint size)
{
	DB( g_print("da_split_new\n") );
	return g_ptr_array_new_full(size, (GDestroyNotify)da_split_free);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static gint da_splits_compare_func(gpointer a, gpointer b)
{
Split *sa = *(Split **)a;
Split *sb = *(Split **)b;	

	DB( g_print(" sort %d %s - %d %s\n", sa->pos, sa->memo, sb->pos, sb->memo) );

	return sa->pos - sb->pos;
}

void da_splits_sort(GPtrArray *splits)
{
	DB( g_print("da_splits_sort\n") );

	if(splits == NULL)
	{
		//g_warning("NULL splits");
		return;
	}

	g_ptr_array_sort(splits, (GCompareFunc) da_splits_compare_func);
}


guint da_splits_length(GPtrArray *splits)
{
	DB( g_print("da_splits_length\n") );

	if(splits == NULL)
	{
		//g_warning("NULL splits");
		return 0;
	}
	
	return splits->len;
}


gboolean da_splits_remove(GPtrArray *splits, Split *item)
{
	DB( g_print("da_splits_remove\n") );

	if(splits == NULL)
	{
		g_warning("NULL splits");
		return FALSE;
	}

	return g_ptr_array_remove(splits, item);
}


void da_splits_append(GPtrArray *splits, Split *item)
{
	DB( g_print("da_splits_append\n") );

	if(splits == NULL)
	{
		g_warning("NULL splits");
		return;
	}

	if(splits->len <= TXN_MAX_SPLIT)
		g_ptr_array_add (splits, item);	
}


Split *da_splits_get(GPtrArray *splits, guint index)
{
	return g_ptr_array_index(splits, index);
}


GPtrArray *da_splits_clone(GPtrArray *src_splits)
{
GPtrArray *new_splits;
guint i;

	DB( g_print("da_splits_clone\n") );

	if(src_splits == NULL)
	{
		//g_warning("NULL splits");
		return NULL;
	}

	new_splits = da_split_new_full (src_splits->len);
	for(i=0;i<src_splits->len;i++)
	{
	Split *src, *new;
	
		src = g_ptr_array_index(src_splits, i);
		new = da_split_malloc ();

		new->kcat = src->kcat;
		new->memo = g_strdup(src->memo);		
		new->amount = src->amount;
		da_splits_append (new_splits, new);
	}	
	return new_splits;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


guint da_splits_parse(GPtrArray *splits, gchar *cats, gchar *amounts, gchar *memos)
{
gchar **cat_a, **amt_a, **mem_a;
guint count, i;
Split *split;

	if(splits == NULL)
		return 0;

	DB( g_print(" split parse %s :: %s :: %s\n", cats, amounts, memos) );

	cat_a = g_strsplit (cats, "||", 0);
	amt_a = g_strsplit (amounts, "||", 0);
	mem_a = g_strsplit (memos, "||", 0);

	count = g_strv_length(amt_a);
	if( (count == g_strv_length(cat_a)) && (count == g_strv_length(mem_a)) )
	{
		for(i=0;i<count;i++)
		{
			split = da_split_malloc();
			split->kcat = atoi(cat_a[i]);
			split->memo = g_strdup(mem_a[i]);
			split->amount = g_ascii_strtod(amt_a[i], NULL);
			da_splits_append (splits, split);
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


guint da_splits_tostring(GPtrArray *splits, gchar **cats, gchar **amounts, gchar **memos)
{
guint i;
Split *split;
char buf[G_ASCII_DTOSTR_BUF_SIZE];
GString *cat_a, *amt_a , *mem_a;

	if(splits == NULL)
		return 0;

	DB( g_print(" splits tostring\n") );

	cat_a = g_string_new (NULL);
	amt_a = g_string_new (NULL);
	mem_a = g_string_new (NULL);

	for(i=0;i<splits->len;i++)
	{
		split = g_ptr_array_index(splits, i);
		g_string_append_printf (cat_a, "%d", split->kcat);
		g_string_append(amt_a, g_ascii_dtostr (buf, sizeof (buf), split->amount) );
		g_string_append(mem_a, split->memo);

		if((i+1) < splits->len)
		{
			g_string_append(cat_a, "||");
			g_string_append(amt_a, "||");
			g_string_append(mem_a, "||");
		}		
	}	

	*cats = g_string_free(cat_a, FALSE);
	*amounts = g_string_free(amt_a, FALSE);
	*memos = g_string_free(mem_a, FALSE);
	
	return i;
}


guint da_splits_consistency (GPtrArray *splits)
{
Split *split;
guint i;
	
	if(splits == NULL)
		return 0;

	// check split category #1340142
	for(i=0;i<splits->len;i++)
	{
		split = g_ptr_array_index(splits, i);
		if(da_cat_get(split->kcat) == NULL)
		{
			g_warning("split consistency: fixed invalid split cat %d", split->kcat);
			split->kcat = 0;
		}
	}
	return splits->len;
}

