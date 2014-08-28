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

#ifndef __HB_TRANSACTION_GTK_H__
#define __HB_TRANSACTION_GTK_H__

enum {
	HID_AMOUNT,
	MAX_HID_AMOUNT
};

enum {
	TXN_SPLIT_NEW,
	TXN_SPLIT_AMOUNT
};


struct deftransaction_data
{
	GtkWidget	*window;

	GtkWidget	*PO_date;
	GtkWidget	*PO_pay;
	GtkWidget	*PO_arc;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_amount, *BT_split;
	GtkWidget	*CM_valid;
	GtkWidget	*CM_remind;
	GtkWidget	*CM_cheque;

	GtkWidget	*NU_mode;
	GtkWidget	*ST_info;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*LB_accto, *PO_accto;
	GtkWidget	*ST_tags;

	gint	action;
	gint	accnum;
	gint	type;

	Transaction *ope;

};

struct ui_txn_split_dialog_data
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

	Transaction *ope;
	gdouble		amount;
	gdouble		sumsplit;
	gdouble		remsplit;

	gint		nbsplit;
	gint		splittype;
	gint		activeline;
	
	gulong		handler_id[TXN_MAX_SPLIT];
};



GtkWidget *create_deftransaction_window (GtkWindow *parent, gint type);
void deftransaction_set_amount(GtkWidget *widget, gdouble amount);
void deftransaction_set_transaction(GtkWidget *widget, Transaction *ope);
void deftransaction_get			(GtkWidget *widget, gpointer user_data);
void deftransaction_add			(GtkWidget *widget, gpointer user_data);
void deftransaction_dispose(GtkWidget *widget, gpointer user_data);
void deftransaction_set_amount_from_split(GtkWidget *widget, gdouble amount);

#endif
