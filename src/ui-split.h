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

#ifndef __HB_SPLIT_GTK_H__
#define __HB_SPLIT_GTK_H__

#include "ui-transaction.h"
#include "hb-split.h"

enum {
	TXN_SPLIT_NEW,
	TXN_SPLIT_AMOUNT
};


struct ui_split_dialog_data
{
	GtkWidget	*dialog;
	GtkWidget	*BT_rem[TXN_MAX_SPLIT];
	GtkWidget	*BT_add[TXN_MAX_SPLIT];
	GtkWidget	*PO_cat[TXN_MAX_SPLIT];
	GtkWidget	*ST_amount[TXN_MAX_SPLIT];
	GtkWidget	*ST_memo[TXN_MAX_SPLIT];

	GtkWidget	*LB_sumsplit;
	GtkWidget	*LB_remain;
	GtkWidget	*LB_txnamount;

	//Transaction *ope;
	Split		**splits;
	
	gdouble		amount;
	gdouble		sumsplit;
	gdouble		remsplit;

	gint		nbsplit;
	gint		splittype;
	gint		activeline;
	
	gulong		handler_id[TXN_MAX_SPLIT];
};


void ui_split_dialog_line_sensitive(guint line, gboolean sensitive, gpointer user_data);
void ui_split_dialog_compute(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_inactiveline(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_activeline(GtkWidget *widget, gpointer user_data);
void ui_split_dialog_get(struct ui_split_dialog_data *data);
void ui_split_dialog_set(struct ui_split_dialog_data *data);
GtkWidget *ui_split_dialog (GtkWidget *parent, Split *ope_splits[], gdouble amount, void (update_callbackFunction(GtkWidget*, gdouble)));

#endif