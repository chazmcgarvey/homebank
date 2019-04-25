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

#ifndef __HB_SPLIT_GTK_H__
#define __HB_SPLIT_GTK_H__

#include "ui-transaction.h"
#include "hb-split.h"


struct ui_split_dialog_data
{
	GtkWidget	*dialog;
	
	GtkWidget	*LV_split;
	GtkWidget	*PO_cat;
	GtkWidget	*ST_amount;
	GtkWidget	*ST_memo;
	GtkWidget	*BT_edit;
	GtkWidget	*BT_rem;
	GtkWidget	*BT_remall;
	GtkWidget	*BT_add;
	GtkWidget	*BT_apply;
	GtkWidget	*BT_cancel;

	GtkWidget	*IM_edit;
	GtkWidget	*LB_sumsplit;
	GtkWidget	*LB_remain;
	GtkWidget	*LB_txnamount;

	//Transaction *ope;
	GPtrArray	*src_splits;
	GPtrArray	*tmp_splits;

	gdouble		amount;
	gdouble		sumsplit;
	gdouble		remsplit;

	gboolean	isedited;
	gint		nbsplit;
	gint		activeline;

	gulong		hid_cat;
	gulong		hid_amt;
	
};


void ui_split_dialog_line_sensitive(guint line, gboolean sensitive, gpointer user_data);
void ui_split_dialog_compute(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_inactiveline(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_activeline(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_get(struct ui_split_dialog_data *data);
void ui_split_dialog_set(struct ui_split_dialog_data *data);
GtkWidget *ui_split_dialog (GtkWidget *parent, GPtrArray **src_splits, gdouble amount, void (update_callbackFunction(GtkWidget*, gdouble)));

#endif
