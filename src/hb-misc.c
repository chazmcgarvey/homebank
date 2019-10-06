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


//nota: this file should be renamed hb-utils

#include "homebank.h"
#include "hb-misc.h"

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

static const double fac[9] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

double hb_amount_round(const double x, unsigned int digits)
{
	digits = MAX(digits, 8);
    return floor((x * fac[digits]) + 0.5) / fac[digits];
}


// used to convert from national to euro currency
// used in hb_account.c :: only when convert account to euro
// round option is to 0.5 case so 1.32 is 1.3, but 1.35 is 1.4

gdouble hb_amount_to_euro(gdouble amount)
{
	return hb_amount_round((amount * PREFS->euro_value), PREFS->minor_cur.frac_digits);
}


/* new >5.1 currency fct
*
 *	convert an amount in base currency
 *
 */
gdouble hb_amount_base(gdouble value, guint32 kcur)
{
gdouble newvalue;
Currency *cur;

	if(kcur == GLOBALS->kcur)
		return value;

	cur = da_cur_get(kcur);
	if(cur == NULL || cur->rate == 0.0)
		return 0;

	newvalue = value / cur->rate;
	return hb_amount_round(newvalue, cur->frac_digits);
}


static Currency *hb_strfmon_check(gchar *outstr, guint32 kcur)
{
Currency *cur = da_cur_get(kcur);

	if(cur == NULL)
		g_stpcpy(outstr, "nan");
	return cur;
}


gchar *hb_str_rate(gchar *outstr, gint outlen, gdouble rate)
{
gint count, i;
gchar *p;
	
	count = g_snprintf(outstr, outlen, "%.6f", rate);
	//remove trailing 0 and decimal point
	p = &outstr[count-1];
	for(i=count;i>0;i--)
	{
		if(*p == '0')
			*p = '\0';
		else
			break;
		p--;
	}
	if(*p == '.' || *p == ',')
		*p = '\0';

	return outstr;
}

/* this function copy a number 99999.99 at s into d and count
 * number of digits for integer part and decimal part
 */
static gchar * _strfnumcopycount(gchar *s, gchar *d, gchar *decchar, gint *plen, gint *pnbint, gint *pnbdec)
{
gint len=0, nbint=0, nbdec=0;

	// sign part
	if(*s == '-') {
		*d++ = *s++;
		len++;
	}
	// integer part
	while(*s != 0 && *s != '.') {
		*d++ = *s++;
		nbint++;
		len++;
	}
	// decimal separator
	if(*s == '.') {
		d = g_stpcpy(d, decchar);
		len++;
		s++;
	}
	// decimal part
	while(*s != 0) {
		*d++ = *s++;
		nbdec++;
		len++;
	}
	// end string | fill external count
	*d = 0;
	*plen = len;
	*pnbint = nbint;
	*pnbdec = nbdec;

	return d;
}

//todo: used only in ui_prefs.c
gchar *hb_str_formatd(gchar *outstr, gint outlen, gchar *buf1, Currency *cur, gboolean showsymbol)
{
gint len, nbd, nbi;
gchar *s, *d, *tmp;

	d = tmp = outstr;
	if(showsymbol && cur->sym_prefix)
	{
		d = g_stpcpy (d, cur->symbol);
		*d++ = ' ';
		tmp = d;
	}
	
	d = _strfnumcopycount(buf1, d, cur->decimal_char, &len, &nbi, &nbd);

	if( cur->grouping_char != NULL && strlen(cur->grouping_char) > 0 )
	{
	gint i, grpcnt;

		s = buf1;
		d = tmp;
		if(*s == '-')
			*d++ = *s++;

		grpcnt = 4 - nbi;
		for(i=0;i<nbi;i++)
		{
			*d++ = *s++;
			if( !(grpcnt % 3) && i<(nbi-1))
			{
				d = g_stpcpy(d, cur->grouping_char);
			}
			grpcnt++;
		}

		if(nbd > 0)
		{
			d = g_stpcpy(d, cur->decimal_char);
			d = g_stpcpy(d, s+1);
		}
		*d = 0;
	}

	if(showsymbol && !cur->sym_prefix)
	{
		*d++ = ' ';
		d = g_stpcpy (d, cur->symbol);
	}

	*d = 0;
	
	return d;
}


void hb_strfmon(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor)
{
gchar formatd_buf[outlen];
Currency *cur;
gdouble monval;

	if(minor == FALSE)
	{
		cur = hb_strfmon_check(outstr, kcur);
		if(cur != NULL)
		{
			monval = hb_amount_round(value, cur->frac_digits);
			g_ascii_formatd(formatd_buf, outlen, cur->format, monval);
			hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
		}
	}
	else
	{
		monval = hb_amount_base(value, kcur);
		monval = hb_amount_to_euro(monval);
		cur = &PREFS->minor_cur;
		g_ascii_formatd(formatd_buf, outlen, cur->format, monval);
		hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
	}

}


void hb_strfmon_int(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor)
{
gchar formatd_buf[outlen];
Currency *cur;
gdouble monval;

	if(minor == FALSE)
	{
		cur = hb_strfmon_check(outstr, kcur);
		if(cur != NULL)
		{
			monval = hb_amount_round(value, cur->frac_digits);
			g_ascii_formatd(formatd_buf, outlen, "%0.f", monval);
			hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
		}
	}
	else
	{
		monval = hb_amount_base(value, kcur);
		monval = hb_amount_to_euro(monval);
		cur = &PREFS->minor_cur;
		g_ascii_formatd(formatd_buf, outlen, cur->format, monval);
		hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
	}

}


void hb_strfnum(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor)
{
gchar formatd_buf[outlen];
Currency *cur;
gdouble monval;

	if(minor == FALSE)
	{
		cur = hb_strfmon_check(outstr, kcur);
		if(cur != NULL)
		{
			monval = hb_amount_round(value, cur->frac_digits);
			g_ascii_formatd(formatd_buf, outlen, "%.2f", monval);
			hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
		}
	}
	else
	{
		cur = &PREFS->minor_cur;
		monval = hb_amount_to_euro(value);
		g_ascii_formatd(formatd_buf, outlen, "%.2f", monval);
		hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
	}

	
}


gchar *get_normal_color_amount(gdouble value)
{
gchar *color = NULL;

	//fix: 400483
	value = hb_amount_round(value, 2);

	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = (value > 0.0) ? PREFS->color_inc : PREFS->color_exp;
	}
	return color;
}


gchar *get_minimum_color_amount(gdouble value, gdouble minvalue)
{
gchar *color = NULL;

	//fix: 400483
	value = hb_amount_round(value, 2);
	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = (value > 0.0) ? PREFS->color_inc : PREFS->color_exp;
		if( value < minvalue)
			color = PREFS->color_warn;
	}
	return color;
}

void hb_label_set_amount(GtkLabel *label, gdouble value, guint32 kcur, gboolean minor)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];

	hb_strfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur, minor);
	gtk_label_set_text(GTK_LABEL(label), strbuffer);

}


/*
** format/color and set a label text with a amount value
*/
void hb_label_set_colvalue(GtkLabel *label, gdouble value, guint32 kcur, gboolean minor)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];
gchar *markuptxt;
gchar *color = NULL;

	hb_strfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, kcur, minor);

	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = get_normal_color_amount(value);

		//g_print("color: %s\n", color);
		
		if(color)
		{
			markuptxt = g_strdup_printf("<span color='%s'>%s</span>", color, strbuffer);
			gtk_label_set_markup(GTK_LABEL(label), markuptxt);
			g_free(markuptxt);
			return;
		}
	}

	gtk_label_set_text(GTK_LABEL(label), strbuffer);

}




/*
** String utility
*/

gint hb_string_compare(gchar *s1, gchar *s2)
{
gint retval = 0;

    if (s1 == NULL || s2 == NULL)
    {
		if (s1 == NULL && s2 == NULL)
			goto end;

		retval = (s1 == NULL) ? -1 : 1;
    }
    else
    {
        retval = strcasecmp(s1, s2);
    }
end:
	return retval;
}


gint hb_string_utf8_strstr(gchar *haystack, gchar *needle, gboolean exact)
{
gint retval = FALSE;

	if( exact )
	{
		if( g_strstr_len(haystack, -1, needle) != NULL )
		{
			DB( g_print(" found case '%s'\n", needle) );
			retval = 1;
		}
	}
	else
	{
	gchar *nchaystack = g_utf8_casefold(haystack, -1);
	gchar *ncneedle   = g_utf8_casefold(needle, -1);

		if( g_strrstr(nchaystack, ncneedle) != NULL )
		{
			DB( g_print(" found nocase '%s'\n", ncneedle) );
			retval = 1;
		}

		g_free(nchaystack);
		g_free(ncneedle);
	}
	return retval;
}





/*
 * compare 2 utf8 string
 */
gint hb_string_utf8_compare(gchar *s1, gchar *s2)
{
gint retval = 0;
gchar *ns1, *ns2;

    if (s1 == NULL || s2 == NULL)
    {
		if (s1 == NULL && s2 == NULL)
			goto end;

		retval = (s1 == NULL) ? -1 : 1;
    }
    else
    {
		//#1325969
		//retval = g_utf8_collate(s1 != NULL ? s1 : "", s2 != NULL ? s2 : "");
		ns1 = g_utf8_normalize(s1, -1, G_NORMALIZE_DEFAULT);
		ns2 = g_utf8_normalize(s2, -1, G_NORMALIZE_DEFAULT);
        retval = strcasecmp(ns1, ns2);
		g_free(ns2);
		g_free(ns1);
    }
end:
	return retval;
}


void hb_string_strip_crlf(gchar *str)
{
gchar *p = str;

	if(str)
	{
		while( *p )
		{
			if( *p == '\n' || *p == '\r')
			{
				*p = '\0';
			}
			p++;
		}
	}
}


void hb_string_replace_char(gchar c, gchar *str)
{
gchar *s = str;
gchar *d = str;

	if(str)
	{
		while( *s )
		{
			if( *s != c )
			{
				*d++ = *s;
			}
			s++;
		}
		*d = 0;
	}
}


gchar *hb_string_copy_jsonpair(gchar *dst, gchar *str)
{

	while( *str!='\0' )
	{
		if( *str=='}' )
			break;

		if( *str==',' )
		{
			*dst = '\0';
			return str + 1;
		}

		if( *str!='{' && *str!='\"' )
		{
			*dst++ = *str;
		}
		str++;
	}
	*dst = '\0';
	return NULL;
}


void hb_string_inline(gchar *str)
{
gchar *s = str;
gchar *d = str;

	if(str)
	{
		while( *s )
		{
			if( !(*s==' ' || *s=='\t' || *s=='\n' || *s=='\r') )
			{
				*d++ = *s;
			}
			s++;
		}
		*d = 0;
	}
}


/*void strip_extra_spaces(char* str) {
  int i,x;
  for(i=x=1; str[i]; ++i)
    if(!isspace(str[i]) || (i>0 && !isspace(str[i-1])))
      str[x++] = str[i];
  str[x] = '\0';
}*/



gchar*
hb_strdup_nobrackets (const gchar *str)
{
  const gchar *s;
  gchar *new_str, *d;
  gsize length;

  if (str)
    {
      length = strlen (str) + 1;
      new_str = g_new (char, length);
      s = str;
      d = new_str;
      while(*s != '\0')
      {
		if( *s != '[' && *s != ']' )
			*d++ = *s;
      	s++;
      }
      *d = '\0';
    }
  else
    new_str = NULL;

  return new_str;
}


/* if we found a . or , within last x digits it might be a dchar */
static gchar hb_string_raw_amount_guess_dchar(const gchar *s, gint len, gshort digit)
{
gint nbc, nbd, i;
gchar gdc='.';

	DB( g_print(" digit=%d maxidx=%d\n", digit, len-digit-1) );
	
	nbc = nbd = 0;
	for(i=len-1;i>=0;i--)
	{
		DB( g_print(" [%2d] '%c' %d %d '%c'\n", i, s[i], nbc, nbd, gdc) );
		//store rightmost ,. within digit-1
		if( i>=(len-digit-1) )
		{
			if(s[i]=='.' || s[i]==',')
				gdc=s[i];
		}
		if(s[i]=='.') nbd++;
		else if(s[i]==',') nbc++;
	}
	if( gdc=='.' && nbd > 1) gdc='?';
	else if( gdc==',' && nbc > 1) gdc='?';
	
	return gdc;
}


gchar *hb_string_dup_raw_amount_clean(const gchar *string, gint digits)
{
gint l;
gchar *new_str, *d;
gchar gdc;
const gchar *p = string;

	l = strlen(string);
	gdc = hb_string_raw_amount_guess_dchar(string, l, digits);

	new_str = d = g_malloc (l+1);
	while(*p)
	{
		if(*p=='-' || g_ascii_isdigit(*p) )
			*d++ = *p;
		else
			if( *p==gdc )
			{
				*d++ = '.';
			}
		p++;
	}
	*d++ = '\0';

	return new_str;	
}


static gboolean
hb_date_parser_get_nums(gchar *string, gint *n1, gint *n2, gint *n3)
{
gboolean retval;
gchar **str_array;

	//DB( g_print("(qif) hb_qif_parser_get_dmy for '%s'\n", string) );

	retval = FALSE;
	if( string )
	{
		str_array = g_strsplit (string, "/", 3);
		if( g_strv_length( str_array ) != 3 )
		{
			g_strfreev (str_array);
			str_array = g_strsplit (string, ".", 3);
			// fix 371381
			//todo test
			if( g_strv_length( str_array ) != 3 )
			{
				g_strfreev (str_array);
				str_array = g_strsplit (string, "-", 3);
			}
		}

		if( g_strv_length( str_array ) == 3 )
		{
			*n1 = atoi(str_array[0]);
			*n2 = atoi(str_array[1]);
			*n3 = atoi(str_array[2]);
			retval = TRUE;
		}

		g_strfreev (str_array);
	}
	return retval;
}


guint32 hb_date_get_julian(gchar *string, gint datefmt)
{
GDate *date;
gint n1, n2, n3, d, m, y;
guint32 julian = 0;

	DB( g_print("\n[utils] hb_date_get_julian\n") );
	
	DB( g_print(" - '%s' dateorder=%d\n", string, datefmt) );	

	if( hb_date_parser_get_nums(string, &n1, &n2, &n3) )
	{
		DB( g_print(" - '%d' '%d' '%d'\n", n1, n2, n3) );

		switch(datefmt)
		{
			case PRF_DATEFMT_MDY:
				d = n2;
				m = n1;
				y = n3;
				break;
			case PRF_DATEFMT_DMY:
				d = n1;
				m = n2;
				y = n3;
				break;
			default:
			case PRF_DATEFMT_YMD:
				d = n3;
				m = n2;
				y = n1;
				break;
		}

		//correct for 2 digits year
		if(y < 1970)
		{
			if(y < 60)
				y += 2000;
			else
				y += 1900;
		}

		if(d <= 31 && m <= 12)
		{
			if( g_date_valid_dmy(d, m, y) )
			{
				date = g_date_new_dmy(d, m, y);
				julian = g_date_get_julian (date);
				g_date_free(date);
			}
		}

		DB( g_print(" > %d %d %d julian=%d\n", d, m, y, julian) );

	}

	return julian;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/

gint hb_filename_type_get_by_extension(gchar *filepath)
{
gint retval = FILETYPE_UNKNOWN;
gint str_len;

	g_return_val_if_fail(filepath != NULL, FILETYPE_UNKNOWN);

	str_len = strlen(filepath);
	if( str_len >= 4 )
	{
		if( strcasecmp(filepath + str_len - 4, ".ofx") == 0)
			retval = FILETYPE_OFX;
		else
		if( strcasecmp(filepath + str_len - 4, ".qif") == 0)
			retval = FILETYPE_QIF;
		else
		if( strcasecmp(filepath + str_len - 4, ".qfx") == 0)
			retval = FILETYPE_OFX;
		else
		if( strcasecmp(filepath + str_len - 4, ".csv") == 0)
			retval = FILETYPE_CSV_HB;
		else
		if( strcasecmp(filepath + str_len - 4, ".xhb") == 0)
			retval = FILETYPE_HOMEBANK;
	}
	return retval;
}


static gchar *hb_filename_new_without_extension(gchar *filename)
{
gchar *lastdot;

	lastdot = g_strrstr (filename, ".");
	if(lastdot != NULL)
	{
		return g_strndup(filename, strlen(filename) - strlen(lastdot));
	}
	return g_strdup(filename);
}


static gint hb_filename_backup_list_sort_func(gchar **a, gchar **b)
{
gint da = atoi( *a  + strlen(*a) - 12);
gint db = atoi( *b  + strlen(*b) - 12);

	return db - da;
}


GPtrArray *hb_filename_backup_list(gchar *filename)
{
gchar *dirname, *basename;
gchar *rawfilename, *pattern;
GDir *dir;
const gchar *tmpname;
GPatternSpec *pspec;
GPtrArray *array;

	DB( g_print("\n[util] filename backup list\n") );

	dirname = g_path_get_dirname(filename);
	basename = g_path_get_basename(filename);

	DB( g_print(" dir='%s' base='%s'\n", dirname, basename) );

	rawfilename = hb_filename_new_without_extension(basename);
	pattern = g_strdup_printf("%s-????????.bak", rawfilename);
	
	pspec = g_pattern_spec_new(pattern);


	DB( g_print(" pattern='%s'\n", pattern) );

	array = g_ptr_array_new_with_free_func(g_free);

	dir = g_dir_open (PREFS->path_hbfile, 0, NULL);
	if (dir)
	{
		while ((tmpname = g_dir_read_name (dir)) != NULL)
		{
		gboolean match;
	
			match = g_pattern_match_string(pspec, tmpname);
			if( match )
			{
				DB( g_print(" %d => '%s'\n", match, tmpname) );
				g_ptr_array_add(array, g_strdup(tmpname));
			}
		}
	}
	g_free(pattern);
	g_dir_close (dir);
	g_pattern_spec_free(pspec);
	g_free(rawfilename);

	g_free(basename);
	g_free(dirname);
	
	g_ptr_array_sort(array, (GCompareFunc)hb_filename_backup_list_sort_func);

	return array;
}


gchar *hb_filename_backup_get_filtername(gchar *filename)
{
gchar *dirname, *basename;
gchar *rawfilename, *pattern;

	DB( g_print("\n[util] filename backup get filtername\n") );

	dirname = g_path_get_dirname(filename);
	basename = g_path_get_basename(filename);

	DB( g_print(" dir='%s' base='%s'\n", dirname, basename) );

	rawfilename = hb_filename_new_without_extension(basename);

	pattern = g_strdup_printf("%s*.[Bb][Aa][Kk]", rawfilename);

	g_free(rawfilename);
	g_free(basename);
	g_free(dirname);
	
	return pattern;
}


gchar *hb_filename_new_for_backup(gchar *filename)
{
gchar *rawfilename, *newfilename;
GDate date;

	DB( g_print("\n[util] filename new for backup\n") );

	rawfilename = hb_filename_new_without_extension(filename);

	g_date_clear(&date, 1);
	g_date_set_julian (&date, GLOBALS->today);

	newfilename = g_strdup_printf("%s-%04d%02d%02d.bak", 
				rawfilename,
				g_date_get_year(&date),
				g_date_get_month(&date),
				g_date_get_day(&date)
				);
	
	g_free(rawfilename);

	DB( g_print(" - '%s' => '%s'\n", filename, newfilename) );

	return newfilename;
}


gchar *hb_filename_new_with_extension(gchar *filename, const gchar *extension)
{
gchar *rawfilename, *newfilename;

	DB( g_print("\n[util] filename new with extension\n") );

	rawfilename = hb_filename_new_without_extension(filename);
	newfilename = g_strdup_printf("%s.%s", rawfilename, extension);
	g_free(rawfilename);

	DB( g_print(" - '%s' => '%s'\n", filename, newfilename) );

	return newfilename;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


gboolean hb_string_isdate(gchar *str)
{
gint d, m, y;

	return(hb_date_parser_get_nums(str, &d, &m, &y));
}


gboolean hb_string_isdigit(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isdigit(*str++);
	return valid;
}

/*
gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isprint(*str++);
	return valid;
}
*/



gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
gchar *p;
gunichar c;

	if(g_utf8_validate(str, -1, NULL))
	{
		p = str;
		while(*p && valid)
		{
			c = g_utf8_get_char(p);
			valid = g_unichar_isprint(c);
			p = g_utf8_next_char(p);
		}
	}
	return valid;
}


gchar *hb_sprint_date(gchar *outstr, guint32 julian)
{
GDate date;

	g_date_clear(&date, 1);
	g_date_set_julian (&date, julian);
	switch(PREFS->dtex_datefmt)
	{
		case PRF_DATEFMT_MDY:
		{
			g_sprintf(outstr, "%02d/%02d/%04d",
				g_date_get_month(&date),
				g_date_get_day(&date),
				g_date_get_year(&date)
				);
		}
		break;
		case PRF_DATEFMT_DMY:
		{
			g_sprintf(outstr, "%02d/%02d/%04d",
				g_date_get_day(&date),
				g_date_get_month(&date),
				g_date_get_year(&date)
				);
		}
		break;
		default:
			g_sprintf(outstr, "%04d/%02d/%02d",
				g_date_get_year(&date),
				g_date_get_month(&date),
				g_date_get_day(&date)
				);
			break;
	}
	return outstr;
}


//used only in DB() macro !!
void hb_print_date(guint32 jdate, gchar *label)
{
gchar buffer1[128];
GDate *date;

	date = g_date_new_julian(jdate);
	g_date_strftime (buffer1, 128-1, "%x", date);
	g_date_free(date);
	g_print(" - %s %s\n", label != NULL ? label:"date is", buffer1);
}



/*
** parse a string an retrieve an iso date (dd-mm-yy(yy) or dd/mm/yy(yy))
**
*/
/* obsolete 4.5
guint32 hb_date_get_julian_parse(gchar *str)
{
gchar **str_array = NULL;
GDate *date;
guint d, m, y;
guint32 julian = GLOBALS->today;

	// try with - separator
	if( g_strrstr(str, "-") != NULL )
	{
		str_array = g_strsplit (str, "-", 3);
	}
	else
	{
		if( g_strrstr(str, "/") != NULL )
		{
			str_array = g_strsplit (str, "/", 3);
		}
	}

	if( g_strv_length( str_array ) == 3 )
	{
		d = atoi(str_array[0]);
		m = atoi(str_array[1]);
		y = atoi(str_array[2]);

		//correct for 2 digits year
		if(y < 1970)
		{
			if(y < 60)
				y += 2000;
			else
				y += 1900;
		}

		//todo: here if month is > 12 then the format is probably mm/dd/yy(yy)
		//or maybe check with g_date_valid_julian(julian)



		date = g_date_new();
		g_date_set_dmy(date, d, m, y);
		julian = g_date_get_julian (date);
		g_date_free(date);

		DB( g_print("date: %s :: %d %d %d :: %d\n", str, d, m, y, julian ) );

	}

	g_strfreev (str_array);

	return julian;
}
*/

/* -------------------- */

#if MYDEBUG == 1

/*
** hex memory dump
*/
#define MAX_DUMP 16
void hex_dump(guchar *ptr, guint length)
{
guchar ascii[MAX_DUMP+4];
guint i,j;

	g_print("**hex_dump - %d bytes\n", length);

	for(i=0;i<length;)
	{
		g_print("%08x: ", (guint)ptr+i);

		for(j=0;j<MAX_DUMP;j++)
		{
			if(i >= length) break;

			//store ascii value
			if(ptr[i] >= 32 && ptr[i] <= 126)
				ascii[j] = ptr[i];
			else
				ascii[j] = '.';

			g_print("%02x ", ptr[i]);
			i++;
		}
		//newline
		ascii[j] = 0;
		g_print(" '%s'\n", ascii);
	}
}

#endif
