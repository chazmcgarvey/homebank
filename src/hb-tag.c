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
#include "hb-tag.h"

#define MYDEBUG 0

#if MYDEBUG
#define DB(x) (x);
#else
#define DB(x);
#endif

/* our global datas */
extern struct HomeBank *GLOBALS;


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

void da_tag_free(Tag *item)
{
	DB( g_print("da_tag_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item);
	}
}


Tag *da_tag_malloc(void)
{
	DB( g_print("da_tag_malloc\n") );
	return g_malloc0(sizeof(Tag));
}


void da_tag_destroy(void)
{
	DB( g_print("da_tag_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_tag);
}


void da_tag_new(void)
{
	DB( g_print("da_tag_new\n") );
	GLOBALS->h_tag = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_tag_free);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
static void da_tag_max_key_ghfunc(gpointer key, Tag *item, guint32 *max_key)
{
	*max_key = MAX(*max_key, item->key);
}

static gboolean da_tag_name_grfunc(gpointer key, Tag *item, gchar *name)
{
	if( name && item->name )
	{
		if(!strcasecmp(name, item->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_tag_length:
 *
 * Return value: the number of elements
 */
guint da_tag_length(void)
{
	return g_hash_table_size(GLOBALS->h_tag);
}

/**
 * da_tag_remove:
 *
 * delete an tag from the GHashTable
 *
 * Return value: TRUE if the key was found and deleted
 *
 */
gboolean da_tag_remove(guint32 key)
{
	DB( g_print("da_tag_remove %d\n", key) );

	return g_hash_table_remove(GLOBALS->h_tag, &key);
}

/**
 * da_tag_insert:
 *
 * insert an tag into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean da_tag_insert(Tag *item)
{
guint32 *new_key;

	DB( g_print("da_tag_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_tag, new_key, item);

	return TRUE;
}


/**
 * da_tag_append:
 *
 * append a new tag into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean da_tag_append(Tag *item)
{
Tag *existitem;
guint32 *new_key;

	DB( g_print("da_tag_append\n") );

	if( item->name != NULL )
	{
		/* ensure no duplicate */
		//g_strstrip(item->name);
		existitem = da_tag_get_by_name( item->name );
		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_tag_get_max_key() + 1;
			item->key = *new_key;

			DB( g_print(" -> append id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_tag, new_key, item);
			return TRUE;
		}
	}

	DB( g_print(" -> %s already exist: %d\n", item->name, item->key) );

	return FALSE;
}

/**
 * da_tag_get_max_key:
 *
 * Get the biggest key from the GHashTable
 *
 * Return value: the biggest key value
 *
 */
guint32 da_tag_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_tag, (GHFunc)da_tag_max_key_ghfunc, &max_key);
	return max_key;
}


/**
 * da_tag_get_by_name:
 *
 * Get an tag structure by its name
 *
 * Return value: Tag * or NULL if not found
 *
 */
Tag *da_tag_get_by_name(gchar *name)
{
	DB( g_print("da_tag_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_tag, (GHRFunc)da_tag_name_grfunc, name);
}



/**
 * da_tag_get:
 *
 * Get an tag structure by key
 *
 * Return value: Tag * or NULL if not found
 *
 */
Tag *da_tag_get(guint32 key)
{
	DB( g_print("da_tag_get_tag\n") );

	return g_hash_table_lookup(GLOBALS->h_tag, &key);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

guint
tags_count(guint32 *tags)
{
guint count = 0;

	DB( g_print("\n[tags] count\n") );

	if( tags == NULL )
		return 0;

	while(*tags++ != 0 && count < 32)
		count++;

	return count;
}


guint32 *tags_clone(guint32 *tags)
{
guint32 *newtags = NULL;
guint count;

	count = tags_count(tags);
	if(count > 0)
	{
		//1501962: we must also copy the final 0
		newtags = g_memdup(tags, (count+1)*sizeof(guint32));
	}

	return newtags;	
}


static gboolean
tags_key_exists(guint32 *tags, guint32 key)
{
guint count = 0;
	while(*tags != 0 && count < 32)
	{
		if( *tags == key )
			return TRUE;
		tags++;
		count++;
	}
	return FALSE;
}


guint32 *
tags_parse(const gchar *tagstring)
{
gchar **str_array;
guint32 *tags = NULL;
guint32 *ptags;
guint count, i;
Tag *tag;

	DB( g_print("\n[tags] parse\n") );

	if( tagstring )
	{
		str_array = g_strsplit (tagstring, " ", 0);
		count = g_strv_length( str_array );
		DB( g_print("- %d tags '%s'\n", count, tagstring) );
		if( count > 0 )
		{
			tags = g_new0(guint32, count + 1);
			ptags = tags;
			for(i=0;i<count;i++)
			{
				//5.2.3 fixed empty tag
				if( strlen(str_array[i]) == 0 )
					continue;

				DB( g_print("- %d search '%s'\n", i, str_array[i]) );
				tag = da_tag_get_by_name(str_array[i]);
				if(tag == NULL)
				{
				Tag *newtag = da_tag_malloc();

					newtag->name = g_strdup(str_array[i]);
					da_tag_append(newtag);
					tag = da_tag_get_by_name(str_array[i]);
				}
				DB( g_print("- array add %d '%s'\n", tag->key, tag->name) );

				//5.3 fixed duplicate tag in same tags
				if( tags_key_exists(tags, tag->key) == FALSE )
					*ptags++ = tag->key;
			}
			*ptags = 0;
		}

		g_strfreev (str_array);
	}
	return tags;
}



gchar *
tags_tostring(guint32 *tags)
{
guint count, i;
gchar **str_array, **tptr;
gchar *tagstring;
Tag *tag;

	DB( g_print("\n[tags] tostring\n") );
	if( tags == NULL )
	{
		return NULL;
	}
	else
	{
		count = tags_count(tags);
		str_array = g_new0(gchar*, count+1);
		tptr = str_array;
		for(i=0;i<count;i++)
		{
			tag = da_tag_get(tags[i]);
			if( tag )
			{
				*tptr++ = tag->name;
			}
		}
		*tptr = NULL;
		
		tagstring = g_strjoinv(" ", str_array);
		g_free (str_array);
	}
	return tagstring;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


#if MYDEBUG

static void
da_tag_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Tag *item = value;

	DB( g_print(" %d :: %s\n", *id, item->name) );

}

static void
da_tag_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_tag, da_tag_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */


gboolean
tag_rename(Tag *item, const gchar *newname)
{
Tag *existitem;
gchar *stripname;
gboolean retval = FALSE;

	stripname = g_strdup(newname);
	g_strstrip(stripname);

	existitem = da_tag_get_by_name(stripname);

	if( existitem != NULL && existitem->key != item->key)
	{
		DB( g_print("- error, same name already exist with other key %d <> %d\n",existitem->key, item->key) );
		g_free(stripname);
	}
	else
	{
		DB( g_print("- renaming\n") );
		g_free(item->name);
		item->name = stripname;
		retval = TRUE;
	}

	return retval;
}



static gint
tag_glist_name_compare_func(Tag *a, Tag *b)
{
	return hb_string_utf8_compare(a->name, b->name);
}


static gint
tag_glist_key_compare_func(Tag *a, Tag *b)
{
	return a->key - b->key;
}


GList *tag_glist_sorted(gint column)
{
GList *list = g_hash_table_get_values(GLOBALS->h_tag);

	if(column == 0)
		return g_list_sort(list, (GCompareFunc)tag_glist_key_compare_func);
	else
		return g_list_sort(list, (GCompareFunc)tag_glist_name_compare_func);
}





