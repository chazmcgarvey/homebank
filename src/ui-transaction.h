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

#ifndef __HB_TRANSACTION_GTK_H__
#define __HB_TRANSACTION_GTK_H__

#include "ui-split.h"

enum {
	HID_AMOUNT,
	MAX_HID_AMOUNT
};


struct deftransaction_data
{
	GtkWidget	*window;

	/* popover */
	GtkWidget   *MB_template;
	GtkTreeModel *model;
	GtkTreeModelFilter *modelfilter;
	GtkWidget   *LV_arc;
	GtkWidget   *CM_showsched;
	GtkWidget   *ST_search;

	GtkWidget	*PO_date;
	GtkWidget	*PO_pay;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_split;
	GtkWidget	*CM_cheque;

	GtkWidget	*NU_mode;
	GtkWidget	*ST_info;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*LB_accto, *PO_accto;
	GtkWidget	*ST_tags;
	GtkWidget   *RA_status;

	GtkWidget   *IB_warnsign;
	
	gint		action;
	gint		accnum;
	gint		type;
	gboolean	showtemplate;

	Transaction *ope;

};

enum
{
	LST_DSPTPL_DATAS,
	LST_DSPTPL_NAME,
	NUM_LST_DSPTPL
};


GtkWidget *create_deftransaction_window (GtkWindow *parent, gint type, gboolean postmode);
void deftransaction_set_amount(GtkWidget *widget, gdouble amount);
gint deftransaction_external_edit(GtkWindow *parent, Transaction *old_txn, Transaction *new_txn);
void deftransaction_set_transaction(GtkWidget *widget, Transaction *ope);
void deftransaction_get			(GtkWidget *widget, gpointer user_data);
void deftransaction_add			(GtkWidget *widget, gpointer user_data);
void deftransaction_dispose(GtkWidget *widget, gpointer user_data);
void deftransaction_set_amount_from_split(GtkWidget *widget, gdouble amount);

#endif
