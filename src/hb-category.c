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
#include "hb-category.h"


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

Category *
da_cat_clone(Category *src_item)
{
Category *new_item = g_memdup(src_item, sizeof(Category));

	DB( g_print("da_cat_clone\n") );
	if(new_item)
	{
		//duplicate the string
		new_item->name		= g_strdup(src_item->name);
	}
	return new_item;
}


void
da_cat_free(Category *item)
{
	DB( g_print("da_cat_free\n") );
	if(item != NULL)
	{
		DB( g_print(" => %d, %s\n", item->key, item->name) );

		g_free(item->name);
		g_free(item);
	}
}


Category *
da_cat_malloc(void)
{
	DB( g_print("da_cat_malloc\n") );
	return g_malloc0(sizeof(Category));
}


void
da_cat_destroy(void)
{
	DB( g_print("da_cat_destroy\n") );
	g_hash_table_destroy(GLOBALS->h_cat);
}


void
da_cat_new(void)
{
Category *item;

	DB( g_print("da_cat_new\n") );
	GLOBALS->h_cat = g_hash_table_new_full(g_int_hash, g_int_equal, (GDestroyNotify)g_free, (GDestroyNotify)da_cat_free);

	// insert our 'no category'
	item = da_cat_malloc();
	item->name = g_strdup("");
	da_cat_insert(item);
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

/**
 * da_cat_length:
 *
 * Return value: the number of elements
 */
guint
da_cat_length(void)
{
	return g_hash_table_size(GLOBALS->h_cat);
}



/**
 * da_cat_remove_grfunc:
 *
 * GRFunc to get the max id
 *
 * Return value: TRUE if the key/value must be removed
 *
 */
static gboolean
da_cat_remove_grfunc(gpointer key, Category *cat, guint32 *remkey)
{
	if(cat->key == *remkey || cat->parent == *remkey)
		return TRUE;

	return FALSE;
}


/**
 * da_cat_remove:
 *
 * remove a category from the GHashTable
 *
 * Return value: TRUE if the key was found and removed
 *
 */
guint
da_cat_remove(guint32 key)
{
	DB( g_print("da_cat_remove %d\n", key) );

	return g_hash_table_foreach_remove(GLOBALS->h_cat, (GHRFunc)da_cat_remove_grfunc, &key);
}

/**
 * da_cat_insert:
 *
 * insert a category into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_cat_insert(Category *item)
{
guint32 *new_key;

	DB( g_print("da_cat_insert\n") );

	new_key = g_new0(guint32, 1);
	*new_key = item->key;
	g_hash_table_insert(GLOBALS->h_cat, new_key, item);

	return TRUE;
}


/**
 * da_cat_append:
 *
 * append a category into the GHashTable
 *
 * Return value: TRUE if inserted
 *
 */
gboolean
da_cat_append(Category *cat)
{
Category *existitem;
guint32 *new_key;
gchar *fullname;

	DB( g_print("da_cat_append\n") );

	if( cat->name != NULL)
	{

		fullname = da_cat_get_fullname(cat);
		existitem = da_cat_get_by_fullname( fullname );
		g_free(fullname);

		if( existitem == NULL )
		{
			new_key = g_new0(guint32, 1);
			*new_key = da_cat_get_max_key() + 1;
			cat->key = *new_key;

			DB( g_print(" -> insert id: %d\n", *new_key) );

			g_hash_table_insert(GLOBALS->h_cat, new_key, cat);
			return TRUE;
		}

	}

	DB( g_print(" -> %s already exist\n", cat->name) );

	return FALSE;
}


/**
 * da_cat_max_key_ghfunc:
 *
 * GHFunc for biggest key
 *
 */
static void
da_cat_max_key_ghfunc(gpointer key, Category *cat, guint32 *max_key)
{

	*max_key = MAX(*max_key, cat->key);
}

/**
 * da_cat_get_max_key:
 *
 * Get the biggest key from the GHashTable
 *
 * Return value: the biggest key value
 *
 */
guint32
da_cat_get_max_key(void)
{
guint32 max_key = 0;

	g_hash_table_foreach(GLOBALS->h_cat, (GHFunc)da_cat_max_key_ghfunc, &max_key);
	return max_key;
}

/**
 * da_cat_get_fullname:
 *
 * Get category the fullname 'xxxx:yyyyy'
 *
 * Return value: the category fullname (free it with g_free)
 *
 */
gchar *
da_cat_get_fullname(Category *cat)
{
Category *parent;

	if( cat->parent == 0)
		return g_strdup(cat->name);
	else
	{
		parent = da_cat_get(cat->parent);
		if( parent )
		{
			return g_strdup_printf("%s:%s", parent->name, cat->name);
		}
	}

	return NULL;
}


/**
 * da_cat_name_grfunc:
 *
 * GRFunc to get the max id
 *
 * Return value: TRUE if the key/value pair match our name
 *
 */
static gboolean
da_cat_name_grfunc(gpointer key, Category *cat, gchar *name)
{

//	DB( g_print("%s == %s\n", name, cat->name) );
	if( name && cat->name)
	{
		if(!strcasecmp(name, cat->name))
			return TRUE;
	}
	return FALSE;
}

/**
 * da_cat_get_key_by_name:
 *
 * Get a category key by its name
 *
 * Return value: the category key or -1 if not found
 *
 */
guint32
da_cat_get_key_by_name(gchar *name)
{
Category *cat;

	DB( g_print("da_cat_get_key_by_name\n") );

	cat = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_name_grfunc, name);
	if( cat == NULL)
		return -1;

	return cat->key;
}

/**
 * da_cat_get_by_name:
 *
 * Get a category structure by its name
 *
 * Return value: Category * or NULL if not found
 *
 */
Category *
da_cat_get_by_name(gchar *name)
{
	DB( g_print("da_cat_get_by_name\n") );

	return g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_name_grfunc, name);
}


/* fullname i.e. car:refuel */
struct fullcatcontext
{
	guint	parent;
	gchar	*name;
};


static gboolean
da_cat_fullname_grfunc(gpointer key, Category *item, struct fullcatcontext *ctx)
{

	//DB( g_print("'%s' == '%s'\n", ctx->name, item->name) );
	if( item->parent == ctx->parent )
	{
		if(!strcasecmp(ctx->name, item->name))
			return TRUE;
	}
	return FALSE;
}

Category *
da_cat_get_by_fullname(gchar *fullname)
{
struct fullcatcontext ctx;
gchar **typestr;
Category *item = NULL;

	DB( g_print("da_cat_get_by_fullname\n") );

	typestr = g_strsplit(fullname, ":", 2);
	if( g_strv_length(typestr) == 2 )
	{
		ctx.parent = 0;
		ctx.name = typestr[0];
		DB( g_print(" [x:x] try to find the parent : '%s'\n", typestr[0]) );

		Category *parent = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
		if( parent != NULL )
		{
			ctx.parent = parent->key;
			ctx.name = typestr[1];

			DB( g_print(" [x:x] and searching sub %d '%s'\n", ctx.parent, ctx.name) );

			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
		}
	}
	else
	{
		ctx.parent = 0;
		ctx.name = fullname;

		DB( g_print(" [x] try to '%s'\n", fullname) );

		item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
	}

	g_strfreev(typestr);

	DB( g_print(" return value %p\n", item) );

	return item;
}


/**
 * da_cat_append_ifnew_by_fullname:
 *
 * append a category if it is new by fullname
 *
 * Return value:
 *
 */
Category *
da_cat_append_ifnew_by_fullname(gchar *fullname, gboolean imported)
{
struct fullcatcontext ctx;
gchar **typestr;
Category *newcat, *item, *retval = NULL;
guint32 *new_key;

	DB( g_print("da_cat_append_ifnew_by_fullname\n") );

	DB( g_print(" -> fullname: '%s' %d\n", fullname, strlen(fullname)) );

	if( strlen(fullname) > 0 )
	{
		typestr = g_strsplit(fullname, ":", 2);

		/* if we have a subcategory : aaaa:bbb */
		if( g_strv_length(typestr) == 2 )
		{
			ctx.parent = 0;
			ctx.name = typestr[0];
			DB( g_print(" try to find the parent:'%s'\n", typestr[0]) );

			Category *parent = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( parent == NULL )
			{
				DB( g_print(" -> not found\n") );

				// append a new category
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->name = g_strdup(typestr[0]);
				newcat->imported = imported;

				parent = newcat;

				DB( g_print(" -> insert cat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);
			}

			ctx.parent = parent->key;
			ctx.name = typestr[1];
			DB( g_print(" searching %d '%s'\n", ctx.parent, ctx.name) );

			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( item == NULL )
			{
				// append a new subcategory
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->parent = parent->key;
				newcat->name = g_strdup(typestr[1]);
				newcat->imported = imported;

				newcat->flags |= GF_SUB;

				DB( g_print(" -> insert subcat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);

				retval = newcat;
			}
			else
				retval = item;
		}
		/* this a single category : aaaa */
		else
		{
			ctx.parent = 0;
			ctx.name = typestr[0];
			DB( g_print(" searching %d '%s'\n", ctx.parent, ctx.name) );

			item = g_hash_table_find(GLOBALS->h_cat, (GHRFunc)da_cat_fullname_grfunc, &ctx);
			if( item == NULL )
			{
				// append a new category
				new_key = g_new0(guint32, 1);
				*new_key = da_cat_get_max_key() + 1;

				newcat = da_cat_malloc();
				newcat->key = *new_key;
				newcat->name = g_strdup(typestr[0]);
				newcat->imported = imported;

				DB( g_print(" -> insert cat '%s' id: %d\n", newcat->name, newcat->key) );

				g_hash_table_insert(GLOBALS->h_cat, new_key, newcat);

				retval = newcat;
			}
			else
				retval = item;

		}

		g_strfreev(typestr);
	}

	return retval;
}



/**
 * da_cat_get:
 *
 * Get a category structure by key
 *
 * Return value: Category * or NULL if not found
 *
 */
Category *
da_cat_get(guint32 key)
{
	//DB( g_print("da_cat_get\n") );

	return g_hash_table_lookup(GLOBALS->h_cat, &key);
}


void da_cat_consistency(Category *item)
{
gboolean isIncome;

	// ensure type equal for categories and its children
	if(!(item->flags & GF_SUB) && item->key > 0)
	{
		isIncome = (item->flags & GF_INCOME) ? TRUE : FALSE;
		category_change_type(item, isIncome);
	}
	g_strstrip(item->name);
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

#if MYDEBUG

static void
da_cat_debug_list_ghfunc(gpointer key, gpointer value, gpointer user_data)
{
guint32 *id = key;
Category *cat = value;

	DB( g_print(" %d :: %s (parent=%d\n", *id, cat->name, cat->parent) );

}

static void
da_cat_debug_list(void)
{

	DB( g_print("\n** debug **\n") );

	g_hash_table_foreach(GLOBALS->h_cat, da_cat_debug_list_ghfunc, NULL);

	DB( g_print("\n** end debug **\n") );

}

#endif



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

gboolean
category_is_used(guint32 key)
{
GList *lrul, *list;
guint i, nbsplit;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Transaction *entry = list->data;
		if( key == entry->kcat )
			return TRUE;

		// check split category #1340142
		nbsplit = da_transaction_splits_count(entry);
		for(i=0;i<nbsplit;i++)
		{
		Split *split = entry->splits[i];

			if( key == split->kcat )
				return TRUE;
		}

		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if( key == entry->kcat )
			return TRUE;
		list = g_list_next(list);
	}

	//todo: add budget use here
	
	lrul = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if( key == entry->kcat)
			return TRUE;
		list = g_list_next(list);
	}
	g_list_free(lrul);

	return FALSE;
}

void
category_move(guint32 key1, guint32 key2)
{
GList *lrul, *list;
guint i, nbsplit;

	list = g_list_first(GLOBALS->ope_list);
	while (list != NULL)
	{
	Transaction *entry = list->data;
		if(entry->kcat == key1)
		{
			entry->kcat = key2;
			entry->flags |= OF_CHANGED;
		}

		// move split category #1340142
		nbsplit = da_transaction_splits_count(entry);
		for(i=0;i<nbsplit;i++)
		{
		Split *split = entry->splits[i];

			if( split->kcat == key1 )
			{
				split->kcat = key2;
				entry->flags |= OF_CHANGED;
			}
		}

		list = g_list_next(list);
	}

	list = g_list_first(GLOBALS->arc_list);
	while (list != NULL)
	{
	Archive *entry = list->data;
		if(entry->kcat == key1)
		{
			entry->kcat = key2;
		}
		list = g_list_next(list);
	}

	lrul = list = g_hash_table_get_values(GLOBALS->h_rul);
	while (list != NULL)
	{
	Assign *entry = list->data;

		if(entry->kcat == key1)
		{
			entry->kcat = key2;
		}
		list = g_list_next(list);
	}
	g_list_free(lrul);

}


gboolean
category_rename(Category *item, const gchar *newname)
{
Category *parent, *existitem;
gchar *fullname = NULL;
gchar *stripname;
gboolean retval;

	DB( g_print("(category) rename\n") );

	stripname = g_strdup(newname);
	g_strstrip(stripname);

	if( item->parent == 0)
		fullname = g_strdup(stripname);
	else
	{
		parent = da_cat_get(item->parent);
		if( parent )
		{
			fullname = g_strdup_printf("%s:%s", parent->name, stripname);
		}
	}

	DB( g_print(" - search: %s\n", fullname) );

	existitem = da_cat_get_by_fullname( fullname );

	if( existitem != NULL && existitem->key != item->key)
	{
		DB( g_print("error, same name already exist with other key %d <> %d\n",existitem->key, item->key) );
		retval = FALSE;
	}
	else
	{
		DB( g_print(" -renaming\n") );

		g_free(item->name);
		item->name = g_strdup(stripname);
		retval = TRUE;
	}

	g_free(fullname);
	g_free(stripname);

	return retval;
}


static gint category_glist_name_compare_func(Category *c1, Category *c2)
{
gchar *name1, *name2;
gint retval = 0;

	if( c1 != NULL && c2 != NULL )
	{
		name1 = da_cat_get_fullname(c1);
		name2 = da_cat_get_fullname(c2);

		retval = hb_string_utf8_compare(name1, name2);

		g_free(name2);
		g_free(name1);
	}
	return retval;
}


static gint category_glist_key_compare_func(Category *a, Category *b)
{
gint ka, kb, retval = 0;

	if(a->parent == 0 && b->parent == a->key)
		retval = -1;
	else
	if(b->parent == 0 && a->parent == b->key)
		retval = 1;
	else
	{
		ka = a->parent != 0 ? a->parent : a->key;
		kb = b->parent != 0 ? b->parent : b->key;
		retval = ka - kb;
	}


	#if MYDEBUG == 1
	gchar *str;

	if(retval < 0)
		str = "a < b";
	else
	if(retval ==0)
		str = "a = b";
	else
	if(retval > 0)
		str = "a > b";

	DB( g_print("compare a=%2d:%2d to b=%2d:%2d :: %d [%s]\n", a->key, a->parent, b->key, b->parent, retval, str ) );
	#endif

	return retval;
}


GList *category_glist_sorted(gint column)
{
GList *list = g_hash_table_get_values(GLOBALS->h_cat);

	if(column == 0)
		return g_list_sort(list, (GCompareFunc)category_glist_key_compare_func);
	else
		return g_list_sort(list, (GCompareFunc)category_glist_name_compare_func);
}


gboolean
category_load_csv(gchar *filename, gchar **error)
{
gboolean retval;
GIOChannel *io;
gchar *tmpstr;
gint io_stat;
gchar **str_array;
gchar *lastcatname = NULL;
gchar *fullcatname;
GError *err = NULL;
Category *item;
gint type = 0;
const gchar *encoding;

	encoding = homebank_file_getencoding(filename);

			DB( g_print(" -> encoding should be %s\n", encoding) );


	retval = TRUE;
	*error = NULL;
	io = g_io_channel_new_file(filename, "r", NULL);
	if(io != NULL)
	{

		if( encoding != NULL )
		{
			g_io_channel_set_encoding(io, encoding, NULL);
		}

		for(;;)
		{
			if( *error != NULL )
				break;
			io_stat = g_io_channel_read_line(io, &tmpstr, NULL, NULL, &err);

			DB( g_print(" + iostat %d\n", io_stat) );

			if( io_stat == G_IO_STATUS_ERROR )
			{
				DB (g_print(" + ERROR %s\n",err->message));
				break;
			}
			if( io_stat == G_IO_STATUS_EOF)
				break;
			if( io_stat == G_IO_STATUS_NORMAL)
			{
				if( tmpstr != NULL )
				{
					DB( g_print(" + strip %s\n", tmpstr) );

					hb_string_strip_crlf(tmpstr);

					DB( g_print(" + split\n") );

					str_array = g_strsplit (tmpstr, ";", 3);
					// type; sign; name

					if( g_strv_length (str_array) != 3 )
					{
						*error = _("invalid csv format");
						retval = FALSE;
						DB( g_print(" + error %s\n", *error) );
					}
					else
					{
						DB( g_print(" + read %s : %s : %s\n", str_array[0], str_array[1], str_array[2]) );

						fullcatname = NULL;
						if( g_str_has_prefix(str_array[0], "1") )
						{
							fullcatname = g_strdup(str_array[2]);
							g_free(lastcatname);
							lastcatname = g_strdup(str_array[2]);

							type = g_str_has_prefix(str_array[1], "+") ? GF_INCOME : 0;

							DB( g_print(" + type = %d\n", type) );

						}
						else
						if( g_str_has_prefix(str_array[0], "2") )
						{
							fullcatname = g_strdup_printf("%s:%s", lastcatname, str_array[2]);
						}

						DB( g_print(" + fullcatname %s\n", fullcatname) );

						item = da_cat_append_ifnew_by_fullname(fullcatname, FALSE);

						DB( g_print(" + item %p\n", item) );

						if( item != NULL)
						{
							DB( g_print(" + assign flags: '%c'\n", type) );

							item->flags |= type;

						}

						g_free(fullcatname);
						g_strfreev (str_array);
					}
				}

			}
			g_free(tmpstr);

		}
		g_io_channel_unref (io);


	}

	g_free(lastcatname);

	return retval;
}



gboolean
category_save_csv(gchar *filename, gchar **error)
{
gboolean retval = FALSE;
GIOChannel *io;
gchar *outstr;
GList *lcat, *list;


	io = g_io_channel_new_file(filename, "w", NULL);
	if(io != NULL)
	{
		lcat = list = category_glist_sorted(1);

		while (list != NULL)
		{
		Category *item = list->data;

			if(item->key != 0)
			{
			gchar lvel, type;

				if( item->parent == 0)
				{
					lvel = '1';
					type = (item->flags & GF_INCOME) ? '+' : '-';
				}
				else
				{
					lvel = '2';
					type = ' ';
				}

				outstr = g_strdup_printf("%c;%c;%s\n", lvel, type, item->name);

				DB( g_print(" + export %s\n", outstr) );

				g_io_channel_write_chars(io, outstr, -1, NULL, NULL);

				g_free(outstr);
			}
			list = g_list_next(list);
		}

		retval = TRUE;

		g_list_free(lcat);

		g_io_channel_unref (io);
	}


	return retval;
}


gint category_change_type(Category *item, gboolean isIncome)
{
gint changes = 1;
GList *lcat, *list;

	item->flags &= ~(GF_INCOME);	//remove flag
	if(isIncome == TRUE)
		item->flags |= GF_INCOME;

	// change also childs
	lcat = list = g_hash_table_get_values(GLOBALS->h_cat);
	while (list != NULL)
	{
	Category *child = list->data;

		if(child->parent == item->key)
		{
			child->flags &= ~(GF_INCOME);	//remove flag
			if(isIncome == TRUE)
				child->flags |= GF_INCOME;
			changes++;
		}
		list = g_list_next(list);
	}

	g_list_free(lcat);

	return changes;
}





/**
 * category_find_preset:
 *
 * find a user language compatible file for category preset
 *
 * Return value: a pathname to the file or NULL
 *
 */
gchar *category_find_preset(gchar **lang)
{
gchar **langs;
gchar *filename;
gboolean exists;
guint i;

	DB( g_print("** category_find_preset **\n") );

	langs = (gchar **)g_get_language_names ();

	DB( g_print(" -> %d languages detected\n", g_strv_length(langs)) );

	for(i=0;i<g_strv_length(langs);i++)
	{
		DB( g_print(" -> %d '%s'\n", i, langs[i]) );
		filename = g_strdup_printf("hb-categories-%s.csv", langs[i]);
		gchar *pathfilename = g_build_filename(homebank_app_get_datas_dir(), filename, NULL);
		exists = g_file_test(pathfilename, G_FILE_TEST_EXISTS);
		DB( g_print(" -> '%s' exists=%d\n", pathfilename, exists) );
		if(exists)
		{
			g_free(filename);
			*lang = langs[i];
			return pathfilename;
		}
		g_free(filename);
		g_free(pathfilename);
	}

	DB( g_print("return NULL\n") );

	*lang = NULL;
	return NULL;
}


