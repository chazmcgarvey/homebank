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

#ifndef __HB_CATEGORY_H__
#define __HB_CATEGORY_H__


typedef struct _category		Category;


struct _category
{
	guint32		key;
	guint32		parent;
	gushort		flags;
	gushort		_pad1;
	gchar		*name;
	gdouble		budget[13];	//0:is same value, 1 ..12 are months

	/* unsaved datas */
	gchar		*fullname;
	gboolean	flt_select;
	guint		usage_count;
};

#define GF_SUB		(1<<0)
#define GF_INCOME	(1<<1)
#define GF_CUSTOM	(1<<2)
#define GF_BUDGET	(1<<3)
#define GF_FORCED	(1<<4)

Category *da_cat_clone(Category *src_item);
void da_cat_free(Category *item);
Category *da_cat_malloc(void);

void da_cat_destroy(void);
void da_cat_new(void);

guint da_cat_length(void);
guint32 da_cat_get_max_key(void);

guint32 da_cat_remove(guint32 key);
gboolean da_cat_insert(Category *acc);
gboolean da_cat_append(Category *cat);
Category *da_cat_append_ifnew_by_fullname(gchar *rawfullname);

Category *da_cat_get(guint32 key);
gchar *da_cat_get_name(Category *item);
Category *da_cat_get_by_fullname(gchar *rawfullname);

void da_cat_consistency(Category *item);

GList *category_glist_sorted(gint column);

guint32 category_report_id(guint32 key, gboolean subcat);

void category_delete_unused(void);
void category_fill_usage(void);

void category_move(guint32 key1, guint32 key2);
gboolean category_rename(Category *item, const gchar *newname);

gchar *category_find_preset(gchar **lang);
gint category_type_get(Category *item);
gchar category_get_type_char(Category *item);
gint category_change_type(Category *item, gboolean isIncome);

gboolean category_load_csv(gchar *filename, gchar **error);
gboolean category_save_csv(gchar *filename, gchar **error);

#endif
