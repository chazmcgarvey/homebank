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

#ifndef __HB_WIDGETS_GTK_H__
#define __HB_WIDGETS_GTK_H__


GtkWidget *make_label(gchar *str, gfloat xalign, gfloat yalign);
GtkWidget *make_clicklabel(gchar *id, gchar *str);
GtkWidget *make_label_group(gchar *str);
GtkWidget *make_label_widget(gchar *str);
GtkWidget *make_text(gfloat xalign);
GtkWidget *make_search(GtkWidget *label);
GtkWidget *make_string(GtkWidget *label);
GtkWidget *make_image_button(gchar *icon_name, gchar *tooltip_text);

GtkWidget *make_memo_entry(GtkWidget *label);
GtkWidget *make_string_maxlength(GtkWidget *label, guint max_length);
GtkWidget *make_amount(GtkWidget *label);
GtkWidget *make_exchange_rate(GtkWidget *label);
GtkWidget *make_numeric(GtkWidget *label, gdouble min, gdouble max);
GtkWidget *make_scale(GtkWidget *label);
GtkWidget *make_long(GtkWidget *label);
GtkWidget *make_year(GtkWidget *label);
GtkWidget *make_cycle(GtkWidget *label, gchar **items);
GtkWidget *make_daterange(GtkWidget *label, gboolean custom);


void ui_label_set_integer(GtkLabel *label, gint value);

GtkWidget *make_radio(gchar **items, gboolean buttonstyle, GtkOrientation orientation);
GtkWidget *radio_get_nth_widget (GtkContainer *container, gint nth);
gint radio_get_active (GtkContainer *container);
void radio_set_active (GtkContainer *container, gint active);

void
gimp_label_set_attributes (GtkLabel *label,
                           ...);

void hb_widget_visible(GtkWidget *widget, gboolean visible);
void ui_gtk_entry_set_text(GtkWidget *widget, gchar *text);
void ui_gtk_entry_replace_text(GtkWidget *widget, gchar **storage);

guint make_popaccount_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popaccount(GtkWidget *label);

guint make_poppayee_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poppayee(GtkWidget *label);

guint make_poparchive_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poparchive(GtkWidget *label);

guint make_popcategory_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popcategory(GtkWidget *label);


gchar *get_paymode_icon_name(gint index);
GtkWidget *make_paymode(GtkWidget *label);
GtkWidget *make_paymode_nointxfer(GtkWidget *label);
GtkWidget *make_nainex(GtkWidget *label);

#endif
