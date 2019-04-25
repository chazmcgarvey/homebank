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

#ifndef __HB_TAG_GTK_H__
#define __HB_TAG_GTK_H__


enum
{
	LST_DEFTAG_TOGGLE,
	LST_DEFTAG_DATAS,
	NUM_LST_DEFTAG
};


struct ui_tag_manage_dialog_data
{
	GtkWidget	*window;
	
	GtkWidget	*ST_name;
	GtkWidget	*LV_tag;

	GtkWidget	*BT_add;
	GtkWidget	*BT_edit;
	GtkWidget	*BT_delete;

	gint		change;
};


struct ui_tag_dialog_data
{
	GtkWidget	*dialog;

};

/* = = = = = = = = = = */


void ui_tag_combobox_populate(GtkComboBoxText *combobox);
GtkWidget *ui_tag_combobox_new(GtkWidget *label);


/* = = = = = = = = = = */

void ui_tag_listview_add(GtkTreeView *treeview, Tag *item);
guint32 ui_tag_listview_get_selected_key(GtkTreeView *treeview);
void ui_tag_listview_remove_selected(GtkTreeView *treeview);
void ui_tag_listview_populate(GtkWidget *view, gint insert_type);
GtkWidget *ui_tag_listview_new(gboolean withtoggle);

GtkWidget *ui_tag_manage_dialog (void);


#endif