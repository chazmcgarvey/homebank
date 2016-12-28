/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2016 Maxime DOYEN
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

static const double fac[7] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

double hb_amount_round(const double x, unsigned int digits)
{
	digits = MAX(digits, 6);
    return floor((x * fac[digits]) + 0.5) / fac[digits];
}


// used to convert from national to euro currency
// used in hb_account.c :: only when convert account to euro
// round option is to 0.5 case so 1.32 is 1.3, but 1.35 is 1.4

gdouble hb_amount_to_euro(gdouble amount)
{
	return hb_amount_round((amount / PREFS->euro_value), 2);
	//return hb_amount_round((amount * PREFS->euro_value), PREFS.minor_cur->frac_digits);
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
		cur = &PREFS->minor_cur;
		monval = hb_amount_to_euro(value);
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
		cur = &PREFS->minor_cur;
		monval = hb_amount_to_euro(value);
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
			g_ascii_formatd(formatd_buf, outlen, "%0.f", monval);
			hb_str_formatd(outstr, outlen, formatd_buf, cur, TRUE);
		}
	}
	else
	{
		cur = &PREFS->minor_cur;
		monval = hb_amount_to_euro(value);
		g_ascii_formatd(formatd_buf, outlen, "%0.f", monval);
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


/*static void strip_extra_spaces(char* str) {
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


gchar *hb_util_filename_new_with_extension(gchar *filename, const gchar *extension)
{
gchar *lastdot, *fwe;
gchar *newfilename;

	DB( g_print("\n[util] filename with extension\n") );

	DB( g_print(" - orig: '%s' => '%s'\n", filename, extension) );

	//duplicate without extensions
	lastdot = g_strrstr(filename, ".");
	if(lastdot != NULL)
	{
		fwe = g_strndup(filename, strlen(filename) - strlen(lastdot));
		DB( g_print(" - fwe: '%s'\n", fwe) );
		newfilename = g_strdup_printf("%s.%s", fwe, extension);
		g_free(fwe);
	}
	else
	{
		newfilename = g_strdup_printf("%s.%s", filename, extension);
	}

	DB( g_print(" - new: '%s'\n", newfilename) );

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
