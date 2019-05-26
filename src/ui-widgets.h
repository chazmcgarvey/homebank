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

#ifndef __HB_WIDGETS_GTK_H__
#define __HB_WIDGETS_GTK_H__


typedef struct _hbtk_kv_data		HbKvData;
typedef struct _hbtk_kiv_data		HbKivData;

struct _hbtk_kv_data {
	guint32			key;
	const gchar		*name;
};



#define HBTK_IS_SEPARATOR -66


enum {
	DATE_RANGE_CUSTOM_SHOW,
	DATE_RANGE_CUSTOM_HIDE,
	DATE_RANGE_CUSTOM_DISABLE
};


GtkWidget *make_label(gchar *str, gfloat xalign, gfloat yalign);
GtkWidget *make_clicklabel(gchar *id, gchar *str);
GtkWidget *make_label_group(gchar *str);
GtkWidget *make_label_widget(gchar *str);
GtkWidget *make_text(gfloat xalign);
GtkWidget *make_search(void);
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
GtkWidget *make_daterange(GtkWidget *label, guint dspmode);


void ui_label_set_integer(GtkLabel *label, gint value);


gchar *hbtk_get_label(HbKvData *kvdata, guint32 key);
guint32 hbtk_combo_box_get_active_id (GtkComboBoxText *combobox);
void hbtk_combo_box_set_active_id (GtkComboBoxText *combobox, guint32 active_id);
void hbtk_combo_box_text_append (GtkComboBoxText *combobox, guint32 key, gchar *text);
GtkWidget *hbtk_combo_box_new (GtkWidget *label);
GtkWidget *hbtk_combo_box_new_with_data (GtkWidget *label, HbKvData *kvdata);

gint hbtk_radio_button_get_active (GtkContainer *container);
void hbtk_radio_button_set_active (GtkContainer *container, gint active);
GtkWidget *hbtk_radio_button_get_nth (GtkContainer *container, gint nth);
void hbtk_radio_button_unblock_by_func(GtkContainer *container, GCallback c_handler, gpointer data);
void hbtk_radio_button_block_by_func(GtkContainer *container, GCallback c_handler, gpointer data);
void hbtk_radio_button_connect(GtkContainer *container, const gchar *detailed_signal, GCallback c_handler, gpointer data);
GtkWidget *hbtk_radio_button_new (gchar **items, gboolean buttonstyle);


void gimp_label_set_attributes (GtkLabel *label, ...);


void hb_widget_visible(GtkWidget *widget, gboolean visible);
void ui_gtk_entry_tag_name_append(GtkEntry *entry, gchar *tagname);
void ui_gtk_entry_set_text(GtkWidget *widget, gchar *text);
void ui_gtk_entry_replace_text(GtkWidget *widget, gchar **storage);

/*
guint make_popaccount_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popaccount(GtkWidget *label);

guint make_poppayee_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poppayee(GtkWidget *label);

guint make_poparchive_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_poparchive(GtkWidget *label);

guint make_popcategory_populate(GtkComboBox *combobox, GList *srclist);
GtkWidget *make_popcategory(GtkWidget *label);
*/


gchar *get_paymode_icon_name(gint index);


GtkWidget *make_paymode(GtkWidget *label);
GtkWidget *make_paymode_nointxfer(GtkWidget *label);
GtkWidget *make_nainex(GtkWidget *label);

#endif
