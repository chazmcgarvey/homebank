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

#ifndef __HB_MISC__H__
#define __HB_MISC__H__

double hb_amount_round(const double x, unsigned int n);
gdouble hb_amount_base(gdouble value, guint32 kcur);
gdouble hb_amount_to_euro(gdouble amount);

gchar *hb_str_rate(gchar *outstr, gint outlen, gdouble rate);

gchar *hb_str_formatd(gchar *outstr, gint outlen, gchar *buf1, Currency *cur, gboolean showsymbol);

void hb_strfmon(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor);
void hb_strfmon_int(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor);
void hb_strfnum(gchar *outstr, gint outlen, gdouble value, guint32 kcur, gboolean minor);

gchar *hb_util_filename_new_with_extension(gchar *filename, const gchar *extension);

gchar *get_normal_color_amount(gdouble value);
gchar *get_minimum_color_amount(gdouble value, gdouble minvalue);

void hb_label_set_amount(GtkLabel *label, gdouble value, guint32 kcur, gboolean minor);
void hb_label_set_colvalue(GtkLabel *label, gdouble value, guint32 kcur, gboolean minor);

//void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate);
//void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate);

gint hb_string_compare(gchar *s1, gchar *s2);
gint hb_string_utf8_compare(gchar *s1, gchar *s2);

void hb_string_strip_crlf(gchar *str);
void hb_string_replace_char(gchar c, gchar *str);
gchar* hb_strdup_nobrackets (const gchar *str);

gchar *hb_sprint_date(gchar *outstr, guint32 julian);

guint32 hb_date_get_julian(gchar *string, gint datefmt);

gboolean hb_string_isdate(gchar *str);
gboolean hb_string_isdigit(gchar *str);
gboolean hb_string_isprint(gchar *str);


void hb_print_date(guint32 jdate, gchar *label);

void hex_dump(guchar *ptr, guint length);


#endif
