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

static gdouble fint(gdouble amount)
{
gdouble fi;

	modf(amount, &fi);
	return(fi);
}

static unsigned dix_puissance_n(unsigned n)
{
    unsigned i, retval = 1;

    for(i = 0; i < n; i++)
        retval *= 10;

    return retval;
}

double arrondi(const double x, unsigned n)
{
    unsigned N = dix_puissance_n(n);
    return floor((x * N) + 0.5) / N;
}

// new for v4.5

/*
 *	convert an amount in base currency
 *
 */
gdouble to_base_amount(gdouble value, guint32 kcur)
{
/*
gdouble newvalue;
Currency *cur;

	if(kcur == GLOBALS->kcur)
		return value;

	cur = da_cur_get(kcur);
	if(cur == NULL)
		return 0;
	newvalue = value * cur->rate;
	return newvalue;
*/
	return value;
}


/* new currency fct
static gint real_mystrfmoncurr(gchar *outstr, gint outlen, gchar *buf1, Currency *cur)
{
gint size = 0;
gchar groupbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar **str_array;
guint i, length;
gchar *monstr;

	str_array = g_strsplit(buf1, ".", 2);
	monstr = NULL;

	length = strlen(str_array[0]);

	if( cur->grouping_char == NULL || !strlen(cur->grouping_char) )
	{
		monstr = g_strjoinv(cur->decimal_char, str_array);
	}
	else
	{
	gchar *s = str_array[0];
	gchar *d = groupbuf;

		i = 0;
		// avoid the - for negative amount
		if( *s == '-')
		{
			*d++ = *s++;
			length--;
		}

		// do the grouping
		do
		{
			if( i!=0 && (length % 3) == 0 )
			{
			gchar *gc = cur->grouping_char;

				while( *gc )
					*d++ = *gc++;
			}

			*d++ = *s;
			length--;
			i++;
		}
		while (length && *s++ != '\0');
		*d = 0;

		monstr = g_strjoin(cur->decimal_char, groupbuf, str_array[1], NULL);

	}

	//debug
	//g_print("**\nmystrfmon %.2f\n 0=%s\n 1=%s\n [%d]\n", value, str_array[0], str_array[1], length );
	//g_print(" => %s :: %s\n", monstr, groupbuf);

	g_strfreev(str_array);

	// insert our formated number with symbol
	g_snprintf(outstr, outlen, cur->monfmt, monstr);

	g_free(monstr);

	return size;
}


static Currency *hb_strfmon_check(gchar *outstr, guint32 kcur)
{
Currency *cur = da_cur_get(kcur);

	if(cur == NULL)
		g_stpcpy(outstr, "error");
	return cur;
}


void hb_strfmon(gchar *outstr, gint outlen, gdouble value, guint32 kcur)
{
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
Currency *cur;
gdouble monval;

	cur = hb_strfmon_check(outstr, kcur);
	if(cur != NULL)
	{
		monval = arrondi(value, cur->frac_digits);
		g_ascii_formatd(formatd_buf, G_ASCII_DTOSTR_BUF_SIZE-1, cur->format, monval);
		real_mystrfmoncurr(outstr, outlen, formatd_buf, cur);
	}
}


void hb_strfmon_int(gchar *outstr, gint outlen, gdouble value, guint32 kcur)
{
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
Currency *cur;
gdouble monval;

	cur = hb_strfmon_check(outstr, kcur);
	if(cur != NULL)
	{
		monval = arrondi(value, cur->frac_digits);
		g_ascii_formatd(formatd_buf, sizeof (formatd_buf), "%0.f", monval);
		real_mystrfmoncurr(outstr, outlen, formatd_buf, cur);
	}
}

//todo: remove this
// test for currecny choose dialog
void mystrfmoncurrcurr(gchar *outstr, gint outlen, gdouble value, Currency *cur)
{
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval;

	monval = arrondi(value, cur->frac_digits);
	g_ascii_formatd(formatd_buf, G_ASCII_DTOSTR_BUF_SIZE-1, cur->format, monval);
	real_mystrfmoncurr(outstr, outlen, formatd_buf, cur);
}
*/


	

/* obsolete before currencies */
gint real_mystrfmon(gchar *outstr, gint outlen, gchar *buf1, struct CurrencyFmt *cur)
{
gint size = 0;
gchar groupbuf[G_ASCII_DTOSTR_BUF_SIZE];
gchar **str_array;
guint i, length;
gchar *monstr;

	str_array = g_strsplit(buf1, ".", 2);
	monstr = NULL;

	length = strlen(str_array[0]);
	
	if( cur->grouping_char == NULL || !strlen(cur->grouping_char) )
	{
		monstr = g_strjoinv(cur->decimal_char, str_array);
	}
	else
	{
	gchar *s = str_array[0];
	gchar *d = groupbuf;

		i = 0;
		// avoid the - for negative amount
		if( *s == '-')
		{
			*d++ = *s++;
			length--;
		}
		
		// do the grouping
		do
		{
			if( i!=0 && (length % 3) == 0 )
			{
			gchar *gc = cur->grouping_char;
			
				while( *gc )
					*d++ = *gc++;
			}
		
			*d++ = *s;
			length--;
			i++;	
		}
		while (length && *s++ != '\0');
		*d = 0;

		monstr = g_strjoin(cur->decimal_char, groupbuf, str_array[1], NULL);

	}

	//debug
	//g_print("**\nmystrfmon %.2f\n 0=%s\n 1=%s\n [%d]\n", value, str_array[0], str_array[1], length );
	//g_print(" => %s :: %s\n", monstr, groupbuf);

	g_strfreev(str_array);

	// insert our formated number with symbol
	g_snprintf(outstr, outlen, cur->monfmt, monstr);

	g_free(monstr);
	
	return size;
}


gint mystrfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
struct CurrencyFmt *cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval;
gint size;

	cur = minor ? &PREFS->minor_cur : &PREFS->base_cur;

	monval = arrondi(value, cur->frac_digits);

	if(minor == TRUE)
	{
		monval = (value * PREFS->euro_value);
		monval += (monval > 0.0) ? 0.005 : -0.005;
		monval = (fint(monval * 100) / 100);
	}

	//DB( g_print("fmt = %s\n", cur->format) );

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), cur->format, monval);

	size = real_mystrfmon(outstr, outlen, formatd_buf, cur);

	return size;
}





gint mystrfmon_int(gchar *outstr, gint outlen, gdouble value, gboolean minor)
{
struct CurrencyFmt *cur;
gchar formatd_buf[G_ASCII_DTOSTR_BUF_SIZE];
gdouble monval = value;
gint size;

	cur = minor ? &PREFS->minor_cur : &PREFS->base_cur;

	if(minor == TRUE)
	{
		monval = (value * PREFS->euro_value);
		monval += (monval > 0.0) ? 0.005 : -0.005;
		monval = (fint(monval * 100) / 100);
	}

	g_ascii_formatd(formatd_buf, sizeof (formatd_buf), "%0.f", monval);

	size = real_mystrfmon(outstr, outlen, formatd_buf, cur);

	return size;
}




/* end obsolete call */


gchar *get_normal_color_amount(gdouble value)
{
gchar *color = NULL;

	//fix: 400483
	value = arrondi(value, 2);

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
	value = arrondi(value, 2);
	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = (value > 0.0) ? PREFS->color_inc : PREFS->color_exp;
		if( value < minvalue)
			color = PREFS->color_warn;
	}
	return color;
}

void hb_label_set_amount(GtkLabel *label, gdouble value, gboolean minor)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];

	mystrfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, minor);
	gtk_label_set_text(GTK_LABEL(label), strbuffer);

}


/*
** format/color and set a label text with a amount value
*/
void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];
gchar *markuptxt;
gchar *color = NULL;

	mystrfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, minor);

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
void hb_label_set_colvaluecurr(GtkLabel *label, gdouble value, guint32 currkey)
{
gchar strbuffer[G_ASCII_DTOSTR_BUF_SIZE];
gchar *markuptxt;
gchar *color = NULL;

	hb_strfmon(strbuffer, G_ASCII_DTOSTR_BUF_SIZE-1, value, currkey);

	if(value != 0.0 && PREFS->custom_colors == TRUE)
	{
		color = get_normal_color_amount(value);

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
*/


/*
void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate)
{
GDate *date;
guint month, year, qnum;

	if(refdate > *maxdate)
		refdate = *maxdate;

	date  = g_date_new_julian(refdate);
	month = g_date_get_month(date);
	year  = g_date_get_year(date);
	qnum  = ((month-1)/3)+1;

	DB( g_print("m=%d, y=%d, qnum=%d\n", month, year, qnum) );

	switch( range )
	{
		case 0:		// this month
			g_date_set_day(date, 1);
			*mindate = g_date_get_julian(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year)-1);
			*maxdate = g_date_get_julian(date);
			break;
	
		case 1:		// last month
			g_date_set_day(date, 1);
			g_date_subtract_months(date, 1);
			*mindate = g_date_get_julian(date);
			month = g_date_get_month(date);
			year = g_date_get_year(date);
			g_date_add_days(date, g_date_get_days_in_month(month, year)-1);
			*maxdate = g_date_get_julian(date);
			break;

		case 2:		// this quarter
			g_date_set_day(date, 1);
			g_date_set_month(date, (qnum-1)*3+1);
			*mindate = g_date_get_julian(date);
			g_date_add_months(date, 3);
			g_date_subtract_days(date, 1);
			*maxdate = g_date_get_julian(date);
			break;
			
		case 3:		// last quarter
			g_date_set_day(date, 1);
			g_date_set_month(date, (qnum-1)*3+1);
			g_date_subtract_months(date, 3);
			*mindate = g_date_get_julian(date);
			g_date_add_months(date, 3);
			g_date_subtract_days(date, 1);
			*maxdate = g_date_get_julian(date);
			break;
			
		case 4:		// this year
			g_date_set_dmy(date, 1, 1, year);
			*mindate = g_date_get_julian(date);
			g_date_set_dmy(date, 31, 12, year);
			*maxdate = g_date_get_julian(date);
			break;
		
		// separator
	
		case 6:		// last 30 days
			*mindate = refdate - 30;
			*maxdate = refdate;
			break;

		case 7:		// last 60 days
			*mindate = refdate - 60;
			*maxdate = refdate;
			break;

		case 8:		// last 90 days
			*mindate = refdate - 90;
			*maxdate = refdate;
			break;
	
		case 9:		// last 12 months
			g_date_subtract_months(date, 12);
			*mindate = g_date_get_julian(date);
			*maxdate = refdate;
			break;


	}
	g_date_free(date);
}
*/

/*
** String utility
*/


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


static gboolean
hb_date_parser_get_nums(gchar *string, gint *n1, gint *n2, gint *n3)
{
gboolean retval;
gchar **str_array;

	//DB( g_print("(qif) hb_qif_parser_get_dmy for '%s'\n", string) );

	retval = FALSE;
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

	return retval;
}


guint32 hb_date_get_julian(gchar *string, gint datefmt)
{
GDate *date;
gint n1, n2, n3, d, m, y;
guint32 julian = 0;

	DB( g_print("hb_date_get_julian: %s, %d\n", string, datefmt) );
	
	if( hb_date_parser_get_nums(string, &n1, &n2, &n3) )
	{
		DB( g_print("-> %d %d %d\n", n1, n2, n3) );

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

		DB( g_print("-> %d %d %d\n", d, m, y) );

		if(d <= 31 && m <= 12)
		{
			date = g_date_new();
			g_date_set_dmy(date, d, m, y);
			if( g_date_valid (date) )
			{
				julian = g_date_get_julian (date);
			}
			g_date_free(date);
		}
	}

	return julian;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


gchar *hb_filename_new_with_extention(gchar *filename, const gchar *extension)
{
gchar *dirname;
gchar *basename;
gchar *newbasename;
gchar *newfilename;
gchar **str_array;

	dirname  = g_path_get_dirname (filename);
	basename = g_path_get_basename(filename);
	str_array = g_strsplit(basename, ".", 0);
	newbasename = g_strdup_printf("%s.%s", str_array[0], extension);
	newfilename = g_build_filename(dirname, newbasename, NULL);

	g_strfreev(str_array);
	g_free(basename);
	g_free(dirname);
	g_free(newbasename);

	return newfilename;
}


/* file backup, qif export */


/*gchar *homebank_filename_without_extention(gchar *path)
{
gchar *basename;
gchar *newname;
gchar **str_array;

	basename = g_path_get_basename(path);

	str_array = g_strsplit(basename, ".", 0);

	newname = g_strdup(str_array[0]);

	g_strfreev(str_array);
	g_free(basename);

	return newname;
}*/

/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =*/


static gboolean hb_string_isdate(gchar *str)
{
gint d, m, y;

	return(hb_date_parser_get_nums(str, &d, &m, &y));
}


static gboolean hb_string_isdigit(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isdigit(*str++);
	return valid;
}

/*
static gboolean hb_string_isprint(gchar *str)
{
gboolean valid = TRUE;
	while(*str && valid)
		valid = g_ascii_isprint(*str++);
	return valid;
}
*/



static gboolean hb_string_isprint(gchar *str)
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


gboolean hb_string_csv_valid(gchar *str, guint nbcolumns, gint *csvtype)
{
gchar **str_array;
gboolean valid = TRUE;
guint i;
extern int errno;

#if MYDEBUG == 1
gchar *type[5] = { "string", "date", "int", "double" };
gint lasttype;
#endif

	DB( g_print("\n** hb_string_csv_valid: init %d\n", valid) );

	hb_string_strip_crlf(str);
	str_array = g_strsplit (str, ";", 0);

	DB( g_print(" -> length %d, nbcolumns %d\n", g_strv_length( str_array ), nbcolumns) );

	if( g_strv_length( str_array ) != nbcolumns )
	{
		valid = FALSE;
		goto csvend;
	}

	for(i=0;i<nbcolumns;i++)
	{
#if MYDEBUG == 1
		lasttype = csvtype[i];
#endif

		if(valid == FALSE)
		{
			DB( g_print(" -> fail on column %d, type: %s\n", i, type[lasttype]) );
			break;
		}

		DB( g_print(" -> control column %d, type: %d, valid: %d '%s'\n", i, lasttype, valid, str_array[i]) );

		switch( csvtype[i] )
		{
			case CSV_DATE:
				valid = hb_string_isdate(str_array[i]);
				break;
			case CSV_STRING:
				valid = hb_string_isprint(str_array[i]);
				break;
			case CSV_INT:
				valid = hb_string_isdigit(str_array[i]);
				break;
			case CSV_DOUBLE	:
				g_ascii_strtod(str_array[i], NULL);
				//todo : see this errno
				if( errno )
				{
					DB( g_print("errno: %d\n", errno) );
					valid = FALSE;
				}
				break;
		}
	}

csvend:
	g_strfreev (str_array);

	DB( g_print(" --> return %d\n", valid) );

	return valid;
}


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
