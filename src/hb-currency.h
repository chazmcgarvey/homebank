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

#ifndef __HB_CURRENCY_H__
#define __HB_CURRENCY_H__

typedef struct _currency	Currency;
typedef struct _iso4217		Currency4217;


struct _currency
{
	guint32   	key;
	gushort 	flags;
	gchar		*name;
	gchar		*iso_code;
	gboolean	sym_prefix;
	gchar		*symbol;			/* max symbol is 3 digits in unicode but mostly is 1 digit */
	gchar		*decimal_char;
	gchar		*grouping_char;
	gshort		frac_digits;
	gshort		_pad1;
	gdouble		rate;
	guint32		mdate;
	/* unsaved datas */
	gchar		format[8];			/* hold decimal format: '%.xf' */
	gchar		monfmt[32];			/* hold monetary format: 'prefix %s suffix' */
};

// 0 is free
#define CF_CUSTOM		(1<<1)


struct _iso4217
{
	gchar      *curr_iso_code;
	guint	   curr_frac_digit;
	gchar      *curr_dec_char;
	gchar	   *curr_grp_char;
	gboolean   curr_is_prefix;
	gchar      *curr_symbol;
	gchar      *name;
};


void da_cur_free(Currency *item);
Currency *da_cur_malloc(void);
void da_cur_destroy(void);
void da_cur_new(void);

guint da_cur_length(void);
gboolean da_cur_remove(guint32 key);

void da_cur_init_from4217(Currency *cur, Currency4217 *curfmt);
void da_cur_initformat(Currency *item);

gboolean da_cur_insert(Currency *item);
gboolean da_cur_append(Currency *item);
guint32 da_cur_get_max_key(void);

Currency *da_cur_get_by_name(gchar *name);
Currency *da_cur_get_by_iso_code(gchar *iso_code);
Currency *da_cur_get(guint32 key);

gboolean currency_is_used(guint32 key);
Currency *currency_add_from_user(Currency4217 *curfmt);
gboolean currency_online_sync(GError **error);

Currency4217 *iso4217format_get(gchar *code);


#endif

