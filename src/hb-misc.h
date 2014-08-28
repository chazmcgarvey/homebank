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

#ifndef __HB_MISC__H__
#define __HB_MISC__H__

double arrondi(const double x, unsigned n);

gdouble to_base_amount(gdouble value, guint32 kcur);

gint real_mystrfmon(gchar *outstr, gint outlen, gchar *buf1, struct CurrencyFmt *cur);
gint mystrfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor);

//void hb_strfmon(gchar *outstr, gint outlen, gdouble value, guint32 kcur);
//void hb_strfmon_int(gchar *outstr, gint outlen, gdouble value, guint32 kcur);
//gint hb_strfmon(gchar *outstr, gint outlen, gdouble value, gboolean minor);
gint mystrfmon_int(gchar *outstr, gint outlen, gdouble value, gboolean minor);

gchar *hb_filename_new_with_extention(gchar *filename, const gchar *extension);

gchar *get_normal_color_amount(gdouble value);
gchar *get_minimum_color_amount(gdouble value, gdouble minvalue);

void hb_label_set_amount(GtkLabel *label, gdouble value, gboolean minor);
void hb_label_set_colvalue(GtkLabel *label, gdouble value, gboolean minor);
void hb_label_set_colvaluecurr(GtkLabel *label, gdouble value, guint32 currkey);

//void get_period_minmax(guint month, guint year, guint32 *mindate, guint32 *maxdate);
//void get_range_minmax(guint32 refdate, gint range, guint32 *mindate, guint32 *maxdate);

gint hb_string_utf8_compare(gchar *s1, gchar *s2);

void hb_string_strip_crlf(gchar *str);
gchar* hb_strdup_nobrackets (const gchar *str);

gboolean hb_string_csv_valid(gchar *str, guint nbcolumns, gint *csvtype);

guint32 hb_date_get_julian(gchar *string, gint datefmt);

void hb_print_date(guint32 jdate, gchar *label);

void hex_dump(guchar *ptr, guint length);


#endif
