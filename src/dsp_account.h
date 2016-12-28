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

#ifndef __HB_DSPACCOUNT_H__
#define __HB_DSPACCOUNT_H__



struct ui_multipleedit_dialog_data
{
	GtkWidget	*window;

	GtkWidget	*CM_date, *PO_date;
	GtkWidget	*LB_mode, *CM_mode, *NU_mode;
	GtkWidget	*CM_info, *ST_info;
	GtkWidget	*LB_acc, *CM_acc, *PO_acc;
	GtkWidget	*CM_pay, *PO_pay;
	GtkWidget	*CM_cat, *PO_cat;
	GtkWidget	*CM_tags, *ST_tags;
	GtkWidget	*CM_memo, *ST_memo;
	
	GtkTreeView	*treeview;
	gboolean	has_xfer;
};


enum
{
	ACTION_ACCOUNT_ADD,
	ACTION_ACCOUNT_INHERIT,
	ACTION_ACCOUNT_EDIT,
	ACTION_ACCOUNT_MULTIEDIT,
	ACTION_ACCOUNT_NONE,
	ACTION_ACCOUNT_CLEAR,
	ACTION_ACCOUNT_RECONCILE,
	ACTION_ACCOUNT_DELETE,
	ACTION_ACCOUNT_FILTER,
	ACTION_ACCOUNT_CLOSE,
	MAX_ACTION_ACCOUNT
};

enum {
	HID_RANGE,
	HID_TYPE,
	HID_STATUS,
	HID_SEARCH,
	MAX_HID
};

struct register_panel_data
{
	GtkWidget	*window;
	GtkWidget	*TB_bar;
	GtkWidget   *TB_tools;

	GtkWidget	*CY_range;
	GtkWidget	*CY_type;
	GtkWidget	*CY_status;
//	GtkWidget	*CY_month, *NB_year;
	GtkWidget	*BT_reset;
	GtkWidget	*TX_selection;

	GtkWidget   *ST_search;
	
	GtkWidget	*CM_minor;
	GtkWidget	*TX_balance[3];

	GPtrArray   *gpatxn;
	GtkWidget	*LV_ope;

	gint	busy;
	gchar	*wintitle;
	GtkUIManager	*ui;
	GtkActionGroup *actions;


	Transaction *cur_ope;

	guint32		accnum;
	Account		*acc;

	gboolean	do_sort;
	
	/* status counters */
	gint	hidden, total;
	gdouble		totalsum;

	Filter		*filter;

	guint		timer_tag;
	
	gulong		handler_id[MAX_HID];

	//gint change;	/* change shouldbe done directly */

};

#define DEFAULT_DELAY 750           /* Default delay in ms */

GtkWidget *register_panel_window_new(guint32 accnum, Account *acc);
void register_panel_window_init(GtkWidget *widget, gpointer user_data);


#endif /* __HOMEBANK_DSPACCOUNT_H__ */
