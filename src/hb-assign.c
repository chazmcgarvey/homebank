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
#include "hb-assign.h"

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
da_asg_free(Assign *item)
{
	DB( g_print("da_asg_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->text) );

		g_free(item->text);
		g_free(item);
	}
}


Assign *
da_asg_malloc(void)
{
	DB( g_print("da_asg_malloc\n") );
	return g_malloc0(sizeof(Assign));
}


void
da_asg_destroy(void)
{
	DB( g_print("da_asg_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_rul);
}


void
da_asg_new(void)
{
	DB( g_print("da_asg_new\n") );
	GLOBALS->h_rul = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_asg_free);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_asg_max_key_ghfunc(gpointer key, Assign *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_asg_name_grfunc(gpointer key, Assign *item, gchar *name)
{
	if( name && item->text )
	{
		if(!strcasecmp(name, item->text))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_asg_length:
 *
 * Return value: the number of elements
 */
guint
da_asg_length(void)
{
	return g_hash_table_size(GLOBALS->h_rul);
}

/**
 * da_asg_remove:
 *
 * delete an rul from the GHashTable
 *
 * Return value: TRUE if the key was found and deleted
 *
 */
gboolean
da_asg_remove(guint32 key)
{
	DB( g_print("da_asg_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_rul, &key);
}

/**
 * da_asg_insert:
 *
 * insert an rul into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_asg_insert(Assign *item)
{
guint32 *new_key;

	DB( g_print("da_asg_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_rul, new_key, item);

	return TRUE;
}


/**
 * da_asg_append:
 *
 * append a new rul into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_asg_append(Assign *item)
{
Assign *existitem;
guint32 *new_key;

	DB( g_print("da_asg_append\n") );

	DB( g_print(" -> try append: %s\n", item->text) );

	if( item->text != NULL )
	{
		/* ensure no duplicate */
		existitem = da_asg_get_by_name( item->text );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_asg_get_max_key() + 1;
			item->key = *new_key;

			DB( g_print(" -> append id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_rul, new_key, item);
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->text, item->key) );

	return FALSE;
}

/**
 * da_asg_get_max_key:
 *
 * Get the biggest key from the GHashTable
 *
 * Return value: the biggest key value
 *
 */
guint32
da_asg_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_rul, (GHFunc)da_asg_max_key_ghfunc, &max_key);
	return max_key;
}




/**
 * da_asg_get_by_name:
 *
 * Get an rul structure by its name
 *
 * Return value: rul * or NULL if not found
 *
 */
Assign *
da_asg_get_by_name(gchar *name)
{
	DB( g_print("da_asg_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_rul, (GHRFunc)da_asg_name_grfunc, name);
}



/**
 * da_asg_get:
 *
 * Get an rul structure by key
 *
 * Return value: rul * or NULL if not found
 *
 */
Assign *
da_asg_get(guint32 key)
{
	DB( g_print("da_asg_get_rul\n") );

	return g_hash_table_lookup(GLOBALS->h_rul, &key);
}


static gint
assign_glist_key_compare_func(Assign *a, Assign *b)
{
	return a->key - b->key;
}


GList *assign_glist_sorted(gint column)
{
GList *list = g_hash_table_get_values(GLOBALS->h_rul);

	//if(column == 0)
		return g_list_sort(list, (GCompareFunc)assign_glist_key_compare_func);
	//else
	//	return g_list_sort(list, (GCompareFunc)assign_glist_name_compare_func);
}




/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#if MYDEBUG

static void
da_asg_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Assign *item = value;

	DB( g_print(" %d :: %s\n", *id, item->text) );

}

static void
da_asg_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_rul, da_asg_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif

