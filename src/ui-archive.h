/*  HomeBank -- Free, easy, personal accounting for everyone.
 *  Copyright (C) 1995-2017 Maxime DOYEN
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

#ifndef __HB_ARCHIVE_GTK_H__
#define __HB_ARCHIVE_GTK_H__


enum
{
	ARC_TYPE_ALL = 0,
	ARC_TYPE_SCHEDULED,
	ARC_TYPE_TEMPLATE
};

#define LST_DEFARC_SORT_MEMO  1
#define LST_DEFARC_SORT_PAYEE 2

enum {
	HID_ARC_MEMO,
	HID_ARC_VALID,
	HID_ARC_REMIND,
	HID_ARC_MAX
};


struct ui_arc_manage_data
{
	GtkWidget	*window;

	GList	*tmp_list;
	gint	change;
	//guint32	lastkey;
	Archive		*lastarcitem;


	GtkWidget	*RA_type;
	GtkWidget	*LV_arc;

	GtkWidget   *GR_txnleft;
	GtkWidget	*PO_pay;
	GtkWidget	*ST_word;
	GtkWidget	*ST_amount, *BT_split;
	GtkWidget	*GR_cheque;
	GtkWidget	*CM_cheque;
	GtkWidget   *RA_status;
	GtkWidget	*NU_mode;
	GtkWidget	*PO_grp;
	GtkWidget	*PO_acc;
	GtkWidget	*LB_accto, *PO_accto;

	GtkWidget   *LB_schedinsert;
	GtkWidget	*CM_auto;
	GtkWidget	*LB_next, *PO_next;
	GtkWidget	*LB_every, *NB_every;
	GtkWidget   *LB_weekend, *CY_weekend;
	GtkWidget	*CY_unit;
	GtkWidget	*CM_limit;
	GtkWidget	*NB_limit;
	GtkWidget	*LB_posts;
	

	GtkWidget	*BT_add, *BT_rem;

	gulong		handler_id[HID_ARC_MAX];

};


GtkWidget *ui_arc_manage_dialog (Archive *ext_arc);

#endif
